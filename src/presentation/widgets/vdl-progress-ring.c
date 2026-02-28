/*
 * VXAPDownloader - Progress Ring Widget
 * Circular progress indicator with percentage text
 */

#include "vdl-progress-ring.h"
#include <math.h>

struct _VdlProgressRing {
    GtkDrawingArea parent_instance;
    double progress;
};

G_DEFINE_TYPE (VdlProgressRing, vdl_progress_ring, GTK_TYPE_DRAWING_AREA)

static void
vdl_progress_ring_draw (GtkDrawingArea *area, cairo_t *cr,
                         int width, int height, gpointer user_data)
{
    VdlProgressRing *self = VDL_PROGRESS_RING (area);

    double cx = width / 2.0;
    double cy = height / 2.0;
    double radius = MIN(cx, cy) - 4;
    double line_width = 4.0;

    /* Background circle */
    cairo_set_line_width (cr, line_width);
    cairo_set_source_rgba (cr, 1, 1, 1, 0.08);
    cairo_arc (cr, cx, cy, radius, 0, 2 * M_PI);
    cairo_stroke (cr);

    /* Progress arc */
    if (self->progress > 0) {
        double start = -M_PI / 2.0;
        double end = start + (2 * M_PI * self->progress);

        cairo_set_source_rgba (cr, 0.39, 0.40, 0.95, 1.0); /* #6366f1 */
        cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
        cairo_arc (cr, cx, cy, radius, start, end);
        cairo_stroke (cr);
    }

    /* Percentage text */
    char text[8];
    snprintf (text, sizeof(text), "%d%%", (int)(self->progress * 100));

    cairo_set_source_rgba (cr, 1, 1, 1, 0.9);
    cairo_select_font_face (cr, "sans-serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size (cr, radius * 0.45);

    cairo_text_extents_t extents;
    cairo_text_extents (cr, text, &extents);
    cairo_move_to (cr, cx - extents.width / 2, cy + extents.height / 2);
    cairo_show_text (cr, text);
}

static void
vdl_progress_ring_class_init (VdlProgressRingClass *klass) {}

static void
vdl_progress_ring_init (VdlProgressRing *self)
{
    self->progress = 0;
    gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (self), vdl_progress_ring_draw, NULL, NULL);
    gtk_widget_set_size_request (GTK_WIDGET (self), 80, 80);
}

GtkWidget *vdl_progress_ring_new (void)
{
    return GTK_WIDGET (g_object_new (VDL_TYPE_PROGRESS_RING, NULL));
}

void vdl_progress_ring_set_progress (VdlProgressRing *self, double progress)
{
    g_return_if_fail (VDL_IS_PROGRESS_RING (self));
    self->progress = CLAMP(progress, 0.0, 1.0);
    gtk_widget_queue_draw (GTK_WIDGET (self));
}
