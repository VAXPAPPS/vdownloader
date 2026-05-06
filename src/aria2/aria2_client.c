#include "aria2_client.h"
#include <curl/curl.h>
#include <string.h>
#include <stdio.h>

/* ─────────────────────────────── Private ──────────────────────────────── */

struct _Aria2Client {
    gchar  *endpoint;   /* full URL: http://host:port/jsonrpc */
    gchar  *secret;
    guint   req_id;
    gboolean connected;
};

typedef struct {
    gchar *data;
    gsize  size;
} CurlBuf;

static size_t
_curl_write (void *ptr, size_t size, size_t nmemb, CurlBuf *buf)
{
    gsize total = size * nmemb;
    buf->data = g_realloc (buf->data, buf->size + total + 1);
    memcpy (buf->data + buf->size, ptr, total);
    buf->size += total;
    buf->data[buf->size] = '\0';
    return total;
}

/* Perform a synchronous JSON-RPC call. Returns parsed JsonNode or NULL on error. */
static JsonNode *
_rpc_call (Aria2Client *self, const gchar *method, JsonArray *params)
{
    CURL *curl = curl_easy_init ();
    if (!curl) return NULL;

    /* Build request body */
    JsonBuilder *b = json_builder_new ();
    json_builder_begin_object (b);
    json_builder_set_member_name (b, "jsonrpc");  json_builder_add_string_value (b, "2.0");
    json_builder_set_member_name (b, "id");        json_builder_add_int_value (b, ++self->req_id);
    json_builder_set_member_name (b, "method");    json_builder_add_string_value (b, method);
    json_builder_set_member_name (b, "params");
    if (self->secret && *self->secret) {
        /* Prepend token param */
        JsonArray *p = json_array_new ();
        gchar *tok = g_strdup_printf ("token:%s", self->secret);
        json_array_add_string_element (p, tok);
        g_free (tok);
        if (params) {
            for (guint i = 0; i < json_array_get_length (params); i++)
                json_array_add_element (p, json_node_copy (json_array_get_element (params, i)));
        }
        JsonNode *p_node = json_node_new (JSON_NODE_ARRAY);
        json_node_take_array (p_node, p);
        json_builder_add_value (b, p_node);
    } else {
        if (params) {
            JsonNode *params_node = json_node_new (JSON_NODE_ARRAY);
            json_node_take_array (params_node, json_array_ref (params));
            json_builder_add_value (b, params_node);
        } else {
            json_builder_begin_array (b);
            json_builder_end_array (b);
        }
    }
    json_builder_end_object (b);

    JsonNode *root = json_builder_get_root (b);
    JsonGenerator *gen = json_generator_new ();
    json_generator_set_root (gen, root);
    gchar *body = json_generator_to_data (gen, NULL);
    json_node_free (root);
    g_object_unref (gen);
    g_object_unref (b);

    /* Perform request */
    CurlBuf response = {0};
    struct curl_slist *headers = NULL;
    headers = curl_slist_append (headers, "Content-Type: application/json");

    curl_easy_setopt (curl, CURLOPT_URL,            self->endpoint);
    curl_easy_setopt (curl, CURLOPT_POSTFIELDS,     body);
    curl_easy_setopt (curl, CURLOPT_HTTPHEADER,     headers);
    curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION,  _curl_write);
    curl_easy_setopt (curl, CURLOPT_WRITEDATA,      &response);
    curl_easy_setopt (curl, CURLOPT_TIMEOUT,        10L);
    curl_easy_setopt (curl, CURLOPT_CONNECTTIMEOUT, 5L);

    CURLcode res = curl_easy_perform (curl);
    curl_slist_free_all (headers);
    curl_easy_cleanup (curl);
    g_free (body);

    if (res != CURLE_OK) {
        g_free (response.data);
        self->connected = FALSE;
        return NULL;
    }
    self->connected = TRUE;

    /* Parse response */
    JsonParser *parser = json_parser_new ();
    GError *err = NULL;
    if (!json_parser_load_from_data (parser, response.data, (gssize)response.size, &err)) {
        g_warning ("aria2 JSON parse error: %s", err->message);
        g_error_free (err);
        g_free (response.data);
        g_object_unref (parser);
        return NULL;
    }
    g_free (response.data);

    JsonNode *rjson = json_node_copy (json_parser_get_root (parser));
    g_object_unref (parser);

    /* Extract "result" field */
    if (!JSON_NODE_HOLDS_OBJECT (rjson)) { json_node_free (rjson); return NULL; }
    JsonObject *robj = json_node_get_object (rjson);
    if (!json_object_has_member (robj, "result")) {
        if (json_object_has_member (robj, "error")) {
            JsonObject *e = json_object_get_object_member (robj, "error");
            g_warning ("aria2 RPC error: %s",
                json_object_get_string_member_with_default (e, "message", "unknown"));
        }
        json_node_free (rjson);
        return NULL;
    }
    JsonNode *result = json_node_copy (json_object_get_member (robj, "result"));
    json_node_free (rjson);
    return result;
}

/* ─────────────────────────────── Public ───────────────────────────────── */

Aria2Client *
aria2_client_new (const gchar *host, gint port, const gchar *secret)
{
    curl_global_init (CURL_GLOBAL_DEFAULT);
    Aria2Client *c = g_new0 (Aria2Client, 1);
    c->endpoint = g_strdup_printf ("%s:%d%s", host, port, ARIA2_RPC_PATH);
    c->secret   = g_strdup (secret ? secret : "");
    return c;
}

void
aria2_client_free (Aria2Client *c)
{
    if (!c) return;
    g_free (c->endpoint);
    g_free (c->secret);
    g_free (c);
}

gboolean
aria2_client_is_connected (Aria2Client *c)
{
    return c->connected;
}

gboolean
aria2_client_test_connection (Aria2Client *c)
{
    JsonNode *r = _rpc_call (c, "aria2.getVersion", NULL);
    if (r) { json_node_free (r); return TRUE; }
    return FALSE;
}

void
aria2_client_add_uri (Aria2Client *c, const gchar *url,
                      const gchar *dir, const gchar *out_name, const gchar *referer,
                      gint connections,
                      Aria2Callback cb, gpointer user_data)
{
    JsonArray *params = json_array_new ();

    /* uris array */
    JsonArray *uris = json_array_new ();
    json_array_add_string_element (uris, url);
    JsonNode *uris_node = json_node_new (JSON_NODE_ARRAY);
    json_node_take_array (uris_node, uris);
    json_array_add_element (params, uris_node);

    /* options object */
    JsonObject *opts = json_object_new ();
    if (dir && *dir)
        json_object_set_string_member (opts, "dir", dir);
    if (out_name && *out_name)
        json_object_set_string_member (opts, "out", out_name);
    if (referer && *referer) {
        JsonArray  *hdr_arr = json_array_new ();
        gchar *ref_h = g_strdup_printf ("Referer: %s", referer);
        json_array_add_string_element (hdr_arr, ref_h);
        g_free (ref_h);
        JsonNode *hdr_node = json_node_new (JSON_NODE_ARRAY);
        json_node_take_array (hdr_node, hdr_arr);
        json_object_set_member (opts, "header", hdr_node);
    }
    if (connections > 0) {
        gchar *cs = g_strdup_printf ("%d", connections);
        json_object_set_string_member (opts, "split", cs);
        json_object_set_string_member (opts, "max-connection-per-server", cs);
        g_free (cs);
    }
    JsonNode *opts_node = json_node_new (JSON_NODE_OBJECT);
    json_node_take_object (opts_node, opts);
    json_array_add_element (params, opts_node);

    JsonNode *r = _rpc_call (c, "aria2.addUri", params);
    json_array_unref (params);
    if (cb) cb (c, r, NULL, user_data);
    if (r) json_node_free (r);
}

static void _simple_call (Aria2Client *c, const gchar *method, const gchar *gid,
                           Aria2Callback cb, gpointer ud)
{
    JsonArray *params = json_array_new ();
    json_array_add_string_element (params, gid);
    JsonNode *r = _rpc_call (c, method, params);
    json_array_unref (params);
    if (cb) cb (c, r, NULL, ud);
    if (r) json_node_free (r);
}

void aria2_client_pause       (Aria2Client *c, const gchar *gid, Aria2Callback cb, gpointer ud)
{ _simple_call (c, "aria2.pause",       gid, cb, ud); }
void aria2_client_resume      (Aria2Client *c, const gchar *gid, Aria2Callback cb, gpointer ud)
{ _simple_call (c, "aria2.unpause",     gid, cb, ud); }
void aria2_client_remove      (Aria2Client *c, const gchar *gid, Aria2Callback cb, gpointer ud)
{ _simple_call (c, "aria2.remove",      gid, cb, ud); }
void aria2_client_force_remove(Aria2Client *c, const gchar *gid, Aria2Callback cb, gpointer ud)
{ _simple_call (c, "aria2.forceRemove", gid, cb, ud); }
void aria2_client_remove_download_result(Aria2Client *c, const gchar *gid, Aria2Callback cb, gpointer ud)
{ _simple_call (c, "aria2.removeDownloadResult", gid, cb, ud); }

void aria2_client_pause_all  (Aria2Client *c, Aria2Callback cb, gpointer ud)
{ JsonNode *r = _rpc_call (c, "aria2.pauseAll", NULL); if (cb) cb(c,r,NULL,ud); if(r)json_node_free(r); }
void aria2_client_resume_all (Aria2Client *c, Aria2Callback cb, gpointer ud)
{ JsonNode *r = _rpc_call (c, "aria2.unpauseAll", NULL); if (cb) cb(c,r,NULL,ud); if(r)json_node_free(r); }

DownloadItem *
aria2_client_tell_status (Aria2Client *c, const gchar *gid)
{
    JsonArray *params = json_array_new ();
    json_array_add_string_element (params, gid);
    JsonNode *r = _rpc_call (c, "aria2.tellStatus", params);
    json_array_unref (params);
    if (!r) return NULL;
    DownloadItem *item = NULL;
    if (JSON_NODE_HOLDS_OBJECT (r))
        item = download_item_from_json (json_node_get_object (r));
    json_node_free (r);
    return item;
}

static GList *
_tell_downloads (Aria2Client *c, const gchar *method, JsonArray *params)
{
    JsonNode *r = _rpc_call (c, method, params);
    if (!r) return NULL;
    GList *list = NULL;
    if (JSON_NODE_HOLDS_ARRAY (r)) {
        JsonArray *arr = json_node_get_array (r);
        for (guint i = 0; i < json_array_get_length (arr); i++) {
            JsonObject *obj = json_array_get_object_element (arr, i);
            if (obj) list = g_list_append (list, download_item_from_json (obj));
        }
    }
    json_node_free (r);
    return list;
}

GList *aria2_client_tell_active  (Aria2Client *c)
{ return _tell_downloads (c, "aria2.tellActive", NULL); }

GList *aria2_client_tell_waiting (Aria2Client *c, gint offset, gint num)
{
    JsonArray *p = json_array_new ();
    json_array_add_int_element (p, offset);
    json_array_add_int_element (p, num);
    GList *l = _tell_downloads (c, "aria2.tellWaiting", p);
    json_array_unref (p);
    return l;
}

GList *aria2_client_tell_stopped (Aria2Client *c, gint offset, gint num)
{
    JsonArray *p = json_array_new ();
    json_array_add_int_element (p, offset);
    json_array_add_int_element (p, num);
    GList *l = _tell_downloads (c, "aria2.tellStopped", p);
    json_array_unref (p);
    return l;
}

gboolean
aria2_client_get_global_stat (Aria2Client *c,
                               gint64 *dl_speed, gint64 *ul_speed,
                               gint *num_active, gint *num_waiting, gint *num_stopped)
{
    JsonNode *r = _rpc_call (c, "aria2.getGlobalStat", NULL);
    if (!r) return FALSE;
    if (!JSON_NODE_HOLDS_OBJECT (r)) { json_node_free (r); return FALSE; }
    JsonObject *obj = json_node_get_object (r);
    if (dl_speed)    *dl_speed    = g_ascii_strtoll (json_object_get_string_member_with_default (obj, "downloadSpeed",  "0"), NULL, 10);
    if (ul_speed)    *ul_speed    = g_ascii_strtoll (json_object_get_string_member_with_default (obj, "uploadSpeed",    "0"), NULL, 10);
    if (num_active)  *num_active  = (gint)g_ascii_strtoll (json_object_get_string_member_with_default (obj, "numActive",  "0"), NULL, 10);
    if (num_waiting) *num_waiting = (gint)g_ascii_strtoll (json_object_get_string_member_with_default (obj, "numWaiting", "0"), NULL, 10);
    if (num_stopped) *num_stopped = (gint)g_ascii_strtoll (json_object_get_string_member_with_default (obj, "numStopped", "0"), NULL, 10);
    json_node_free (r);
    return TRUE;
}
