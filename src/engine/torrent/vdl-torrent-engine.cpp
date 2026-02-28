/*
 * VXAPDownloader - BitTorrent Engine (C++ Implementation)
 * Wraps libtorrent-rasterbar 2.x API with a C-compatible interface.
 */

#include "vdl-torrent-engine.h"
#include <libtorrent/session.hpp>
#include <libtorrent/session_params.hpp>
#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_status.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/settings_pack.hpp>
#include <vector>
#include <mutex>
#include <memory>
#include <cstring>
#include <cstdio>
#include <sstream>

struct VdlTorrentEngine {
    std::unique_ptr<lt::session>       session;
    std::vector<lt::torrent_handle>    handles;
    std::mutex                         mutex;
    std::string                        default_save_path;
};

/* Convert libtorrent state to our enum */
static VdlTorrentStatus
convert_state (lt::torrent_status::state_t state, bool paused)
{
    if (paused) return VDL_TORRENT_PAUSED;
    switch (state) {
        case lt::torrent_status::checking_files:
        case lt::torrent_status::checking_resume_data:
            return VDL_TORRENT_CHECKING;
        case lt::torrent_status::downloading_metadata:
            return VDL_TORRENT_DOWNLOADING_META;
        case lt::torrent_status::downloading:
            return VDL_TORRENT_DOWNLOADING;
        case lt::torrent_status::finished:
            return VDL_TORRENT_FINISHED;
        case lt::torrent_status::seeding:
            return VDL_TORRENT_SEEDING;
        default:
            return VDL_TORRENT_ERROR;
    }
}

VdlTorrentEngine *
vdl_torrent_engine_new (const char *save_path)
{
    try {
        auto *engine = new VdlTorrentEngine;

        lt::settings_pack pack;
        pack.set_int (lt::settings_pack::alert_mask,
            lt::alert_category::status | lt::alert_category::error |
            lt::alert_category::storage | lt::alert_category::peer);
        pack.set_str (lt::settings_pack::user_agent, "VXAPDownloader/1.0");
        pack.set_int (lt::settings_pack::connections_limit, 200);
        pack.set_bool(lt::settings_pack::enable_dht, true);
        pack.set_bool(lt::settings_pack::enable_lsd, true);
        pack.set_bool(lt::settings_pack::enable_upnp, true);
        pack.set_bool(lt::settings_pack::enable_natpmp, true);

        engine->session = std::make_unique<lt::session>(pack);
        engine->default_save_path = save_path ? save_path : ".";

        fprintf (stderr, "[torrent] Engine initialized (libtorrent %s)\n",
                 lt::version());

        return engine;
    } catch (const std::exception &e) {
        fprintf (stderr, "[torrent] Failed to init engine: %s\n", e.what());
        return NULL;
    }
}

void
vdl_torrent_engine_destroy (VdlTorrentEngine *engine)
{
    if (!engine) return;
    if (engine->session)
        engine->session->pause ();
    delete engine;
}

int
vdl_torrent_engine_add_torrent_file (VdlTorrentEngine *engine,
                                      const char *torrent_path,
                                      const char *save_path)
{
    if (!engine || !engine->session || !torrent_path) return -1;

    try {
        lt::add_torrent_params params;
        params.ti = std::make_shared<lt::torrent_info>(std::string(torrent_path));
        params.save_path = save_path ? save_path : engine->default_save_path;

        std::lock_guard<std::mutex> lock (engine->mutex);
        lt::torrent_handle handle = engine->session->add_torrent (params);
        engine->handles.push_back (handle);
        int idx = (int)engine->handles.size () - 1;

        fprintf (stderr, "[torrent] Added torrent file: %s (index %d)\n",
                 params.ti->name().c_str(), idx);
        return idx;
    } catch (const std::exception &e) {
        fprintf (stderr, "[torrent] Error adding torrent: %s\n", e.what());
        return -1;
    }
}

int
vdl_torrent_engine_add_magnet (VdlTorrentEngine *engine,
                                const char *magnet_uri,
                                const char *save_path)
{
    if (!engine || !engine->session || !magnet_uri) return -1;

    try {
        lt::add_torrent_params params = lt::parse_magnet_uri (magnet_uri);
        params.save_path = save_path ? save_path : engine->default_save_path;

        std::lock_guard<std::mutex> lock (engine->mutex);
        lt::torrent_handle handle = engine->session->add_torrent (params);
        engine->handles.push_back (handle);
        int idx = (int)engine->handles.size () - 1;

        fprintf (stderr, "[torrent] Added magnet (index %d)\n", idx);
        return idx;
    } catch (const std::exception &e) {
        fprintf (stderr, "[torrent] Error adding magnet: %s\n", e.what());
        return -1;
    }
}

void
vdl_torrent_engine_pause (VdlTorrentEngine *engine, int index)
{
    if (!engine || !engine->session) return;
    std::lock_guard<std::mutex> lock (engine->mutex);
    if (index >= 0 && index < (int)engine->handles.size ()) {
        engine->handles[index].pause ();
    }
}

void
vdl_torrent_engine_resume (VdlTorrentEngine *engine, int index)
{
    if (!engine || !engine->session) return;
    std::lock_guard<std::mutex> lock (engine->mutex);
    if (index >= 0 && index < (int)engine->handles.size ()) {
        engine->handles[index].resume ();
    }
}

void
vdl_torrent_engine_remove (VdlTorrentEngine *engine, int index,
                            bool delete_files)
{
    if (!engine || !engine->session) return;
    std::lock_guard<std::mutex> lock (engine->mutex);
    if (index >= 0 && index < (int)engine->handles.size ()) {
        lt::remove_flags_t flags = {};
        if (delete_files)
            flags = lt::session::delete_files;
        engine->session->remove_torrent (engine->handles[index], flags);
        engine->handles.erase (engine->handles.begin () + index);
    }
}

int
vdl_torrent_engine_count (VdlTorrentEngine *engine)
{
    if (!engine) return 0;
    std::lock_guard<std::mutex> lock (engine->mutex);
    return (int)engine->handles.size ();
}

bool
vdl_torrent_engine_get_info (VdlTorrentEngine *engine, int index,
                              VdlTorrentInfo *info)
{
    if (!engine || !info || !engine->session) return false;
    std::lock_guard<std::mutex> lock (engine->mutex);
    if (index < 0 || index >= (int)engine->handles.size ()) return false;

    try {
        lt::torrent_status ts = engine->handles[index].status ();

        memset (info, 0, sizeof(VdlTorrentInfo));
        strncpy (info->name, ts.name.c_str(), sizeof(info->name) - 1);
        strncpy (info->save_path, ts.save_path.c_str(),
                 sizeof(info->save_path) - 1);

        /* Info hash */
        std::ostringstream oss;
        oss << ts.info_hashes.get_best();
        strncpy (info->info_hash, oss.str().c_str(),
                 sizeof(info->info_hash) - 1);

        info->total_size     = ts.total_wanted;
        info->downloaded     = ts.total_wanted_done;
        info->uploaded       = ts.total_upload;
        info->download_rate  = (double)ts.download_rate;
        info->upload_rate    = (double)ts.upload_rate;
        info->progress       = ts.progress;
        info->num_peers      = ts.num_peers;
        info->num_seeds      = ts.num_seeds;

        /* ETA */
        if (ts.download_rate > 0 && ts.total_wanted > ts.total_wanted_done) {
            info->eta_seconds = (int)((ts.total_wanted - ts.total_wanted_done)
                                       / ts.download_rate);
        } else {
            info->eta_seconds = -1;
        }

        bool paused = (ts.flags & lt::torrent_flags::paused) != lt::torrent_flags_t{};
        info->status = convert_state (ts.state, paused);

        return true;
    } catch (...) {
        return false;
    }
}

double
vdl_torrent_engine_total_download_rate (VdlTorrentEngine *engine)
{
    if (!engine || !engine->session) return 0;
    double total = 0;
    std::lock_guard<std::mutex> lock (engine->mutex);
    for (auto &h : engine->handles) {
        try { total += h.status().download_rate; } catch (...) {}
    }
    return total;
}

double
vdl_torrent_engine_total_upload_rate (VdlTorrentEngine *engine)
{
    if (!engine || !engine->session) return 0;
    double total = 0;
    std::lock_guard<std::mutex> lock (engine->mutex);
    for (auto &h : engine->handles) {
        try { total += h.status().upload_rate; } catch (...) {}
    }
    return total;
}
