/*
 * VXAPDownloader - Speed Graph Widget
 * Realtime speed graph with gradient fill
 */

#include "vdl-speed-graph.h"
#include "../../core/vdl-types.h"
#include <math.h>
#include <string.h>

#define MAX_SAMPLES VDL_SPEED_HISTORY

struct _VdlSpeedGraph {
    GtkDrawingArea parent_instance;

    double   dl_samples[MAX_SAMPLES];
    double   ul_samples[MAX_SAMPLES];
    int      sample_count;
    int      head;
    double   max_speed;
};

G_DEFINE_TYPE (VdlSpeedGraph, vdl_speed_graph, GTK_TYPE_DRAWING_AREA)

static void
draw_line (cairo_t *cr, double *samples, int count, int head,
           double max_speed, int width, int height,
           double r, double g, double b)
{
    if (count < 2 || max_speed <= 0) return;

    double step_x = (double)width / (MAX_SAMPLES - 1);
    int margin = 20;
    double h = height - margin;

    /* Line */
    cairo_set_source_rgba (cr, r, g, b, 0.9);
    cairo_set_line_width (cr, 2.0);

    gboolean first = TRUE;
    for (int i = 0; i < count; i++) {
        int idx = (head - count + 1 + i + MAX_SAMPLES) % MAX_SAMPLES;
        double x = (MAX_SAMPLES - count + i) * step_x;
        double y = margin + h - (samples[idx] / max_speed) * h;

        if (first) {
            cairo_move_to (cr, x, y);
            first = FALSE;
        } else {
            cairo_line_to (cr, x, y);
        }
    }
    cairo_stroke_preserve (cr);

    /* Gradient fill */
    int idx_last = (head) % MAX_SAMPLES;
    double last_x = (MAX_SAMPLES - 1) * step_x;
    (void)idx_last;
    cairo_line_to (cr, last_x, height);

    int idx_first = (head - count + 1 + MAX_SAMPLES) % MAX_SAMPLES;
    double first_x = (MAX_SAMPLES - count) * step_x;
    (void)idx_first;
    cairo_line_to (cr, first_x, height);
    cairo_close_path (cr);

    cairo_pattern_t *grad = cairo_pattern_create_linear (0, margin, 0, height);
    cairo_pattern_add_color_stop_rgba (grad, 0, r, g, b, 0.25);
    cairo_pattern_add_color_stop_rgba (grad, 1, r, g, b, 0.02);
    cairo_set_source (cr, grad);
    cairo_fill (cr);
    cairo_pattern_destroy (grad);
}

static void
vdl_speed_graph_draw (GtkDrawingArea *area, cairo_t *cr,
                       int width, int height, gpointer user_data)
{
    VdlSpeedGraph *self = VDL_SPEED_GRAPH (area);

    /* Background */
    cairo_set_source_rgba (cr, 0, 0, 0, 0);
    cairo_paint (cr);

    /* Grid lines */
    cairo_set_source_rgba (cr, 1, 1, 1, 0.05);
    cairo_set_line_width (cr, 1.0);
    for (int i = 1; i < 4; i++) {
        double y = height * i / 4.0;
        cairo_move_to (cr, 0, y);
        cairo_line_to (cr, width, y);
        cairo_stroke (cr);
    }

    /* Download speed line (blue/purple) */
    draw_line (cr, self->dl_samples, self->sample_count, self->head,
               self->max_speed, width, height,
               0.39, 0.40, 0.95); /* #6366f1 */

    /* Upload speed line (green) */
    draw_line (cr, self->ul_samples, self->sample_count, self->head,
               self->max_speed, width, height,
               0.06, 0.73, 0.50); /* #10b981 */
}

static void
vdl_speed_graph_class_init (VdlSpeedGraphClass *klass) {}

static void
vdl_speed_graph_init (VdlSpeedGraph *self)
{
    memset (self->dl_samples, 0, sizeof(self->dl_samples));
    memset (self->ul_samples, 0, sizeof(self->ul_samples));
    self->sample_count = 0;
    self->head = 0;
    self->max_speed = 1024 * 1024; /* 1 MB/s default */

    gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (self), vdl_speed_graph_draw, NULL, NULL);
    gtk_widget_set_size_request (GTK_WIDGET (self), -1, 150);
}

GtkWidget *vdl_speed_graph_new (void)
{
    return GTK_WIDGET (g_object_new (VDL_TYPE_SPEED_GRAPH, NULL));
}

void vdl_speed_graph_add_sample (VdlSpeedGraph *self, double download_speed, double upload_speed)
{
    g_return_if_fail (VDL_IS_SPEED_GRAPH (self));

    self->head = (self->head + 1) % MAX_SAMPLES;
    self->dl_samples[self->head] = download_speed;
    self->ul_samples[self->head] = upload_speed;

    if (self->sample_count < MAX_SAMPLES)
        self->sample_count++;

    /* Update max */
    double m = 1024 * 1024;
    for (int i = 0; i < self->sample_count; i++) {
        int idx = (self->head - i + MAX_SAMPLES) % MAX_SAMPLES;
        if (self->dl_samples[idx] > m) m = self->dl_samples[idx];
        if (self->ul_samples[idx] > m) m = self->ul_samples[idx];
    }
    self->max_speed = m * 1.2; /* 20% headroom */

    gtk_widget_queue_draw (GTK_WIDGET (self));
}
