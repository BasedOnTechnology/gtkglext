/* GdkGLExt - OpenGL Extension to GDK
 * Copyright (C) 2002  Naofumi Yasufuku
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA.
 */

#include "gdkglprivate.h"
#include "gdkglconfig.h"
#include "gdkglwindow.h"

enum {
  PROP_0,
  PROP_GLCONFIG,
  PROP_WRAPPER
};

static GdkGC *gdk_gl_window_create_gc      (GdkDrawable      *drawable,
                                            GdkGCValues      *values,
                                            GdkGCValuesMask   mask);
static void   gdk_gl_window_draw_rectangle (GdkDrawable      *drawable,
                                            GdkGC            *gc,
                                            gint              filled,
                                            gint              x,
                                            gint              y,
                                            gint              width,
                                            gint              height);
static void   gdk_gl_window_draw_arc       (GdkDrawable      *drawable,
                                            GdkGC            *gc,
                                            gint              filled,
                                            gint              x,
                                            gint              y,
                                            gint              width,
                                            gint              height,
                                            gint              angle1,
                                            gint              angle2);
static void   gdk_gl_window_draw_polygon   (GdkDrawable      *drawable,
                                            GdkGC            *gc,
                                            gint              filled,
                                            GdkPoint         *points,
                                            gint              npoints);
static void   gdk_gl_window_draw_text      (GdkDrawable      *drawable,
                                            GdkFont          *font,
                                            GdkGC            *gc,
                                            gint              x,
                                            gint              y,
                                            const gchar      *text,
                                            gint              text_length);
static void   gdk_gl_window_draw_text_wc   (GdkDrawable      *drawable,
                                            GdkFont          *font,
                                            GdkGC            *gc,
                                            gint              x,
                                            gint              y,
                                            const GdkWChar   *text,
                                            gint              text_length);
static void   gdk_gl_window_draw_drawable  (GdkDrawable      *drawable,
                                            GdkGC            *gc,
                                            GdkDrawable      *src,
                                            gint              xsrc,
                                            gint              ysrc,
                                            gint              xdest,
                                            gint              ydest,
                                            gint              width,
                                            gint              height);
static void   gdk_gl_window_draw_points	   (GdkDrawable      *drawable,
                                            GdkGC            *gc,
                                            GdkPoint         *points,
                                            gint              npoints);
static void   gdk_gl_window_draw_segments  (GdkDrawable      *drawable,
                                            GdkGC            *gc,
                                            GdkSegment       *segs,
                                            gint              nsegs);
static void   gdk_gl_window_draw_lines     (GdkDrawable      *drawable,
                                            GdkGC            *gc,
                                            GdkPoint         *points,
                                            gint              npoints);
static void   gdk_gl_window_draw_glyphs    (GdkDrawable      *drawable,
                                            GdkGC            *gc,
                                            PangoFont        *font,
                                            gint              x,
                                            gint              y,
                                            PangoGlyphString *glyphs);
static void   gdk_gl_window_draw_image     (GdkDrawable      *drawable,
                                            GdkGC            *gc,
                                            GdkImage         *image,
                                            gint              xsrc,
                                            gint              ysrc,
                                            gint              xdest,
                                            gint              ydest,
                                            gint              width,
                                            gint              height);
static gint         gdk_gl_window_get_depth              (GdkDrawable *drawable);
static void         gdk_gl_window_get_size               (GdkDrawable *drawable,
                                                          gint        *width,
                                                          gint        *height);
static void         gdk_gl_window_set_colormap           (GdkDrawable *drawable,
                                                          GdkColormap *cmap);
static GdkColormap* gdk_gl_window_get_colormap           (GdkDrawable *drawable);
static GdkVisual*   gdk_gl_window_get_visual             (GdkDrawable *drawable);
static GdkImage*    gdk_gl_window_get_image              (GdkDrawable *drawable,
                                                          gint         x,
                                                          gint         y,
                                                          gint         width,
                                                          gint         height);
static GdkRegion*   gdk_gl_window_get_clip_region        (GdkDrawable *drawable);
static GdkRegion*   gdk_gl_window_get_visible_region     (GdkDrawable *drawable);
static GdkDrawable* gdk_gl_window_get_composite_drawable (GdkDrawable *drawable,
                                                          gint         x,
                                                          gint         y,
                                                          gint         width,
                                                          gint         height,
                                                          gint        *composite_x_offset,
                                                          gint        *composite_y_offset);
static void         gdk_gl_window_draw_pixbuf   (GdkDrawable *drawable,
                                                 GdkGC       *gc,
                                                 GdkPixbuf   *pixbuf,
                                                 gint         src_x,
                                                 gint         src_y,
                                                 gint         dest_x,
                                                 gint         dest_y,
                                                 gint         width,
                                                 gint         height,
                                                 GdkRgbDither dither,
                                                 gint         x_dither,
                                                 gint         y_dither);
static GdkImage*    gdk_gl_window_copy_to_image (GdkDrawable *drawable,
                                                 GdkImage    *image,
                                                 gint         src_x,
                                                 gint         src_y,
                                                 gint         dest_x,
                                                 gint         dest_y,
                                                 gint         width,
                                                 gint         height);

static void gdk_gl_window_init         (GdkGLWindow      *glwindow);
static void gdk_gl_window_class_init   (GdkGLWindowClass *klass);
static void gdk_gl_window_set_property (GObject          *object,
                                        guint             property_id,
                                        const GValue     *value,
                                        GParamSpec       *pspec);
static void gdk_gl_window_get_property (GObject          *object,
                                        guint             property_id,
                                        GValue           *value,
                                        GParamSpec       *pspec);
static void gdk_gl_window_finalize     (GObject          *object);

static gpointer parent_class = NULL;

GType
gdk_gl_window_get_type (void)
{
  static GType type = 0;

  if (!type)
    {
      static const GTypeInfo type_info = {
        sizeof (GdkGLWindowClass),
        (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc) gdk_gl_window_class_init,
        (GClassFinalizeFunc) NULL,
        NULL,                   /* class_data */
        sizeof (GdkGLWindow),
        0,                      /* n_preallocs */
        (GInstanceInitFunc) gdk_gl_window_init,
      };

      type = g_type_register_static (GDK_TYPE_DRAWABLE,
                                     "GdkGLWindow",
                                     &type_info, 0);
    }

  return type;
}

static void
gdk_gl_window_init (GdkGLWindow *glwindow)
{
  /* 0-initialization is good for all other fields. */

  GDK_GL_NOTE (FUNC, g_message (" - gdk_gl_window_init ()"));
}

static void
gdk_gl_window_class_init (GdkGLWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GdkDrawableClass *drawable_class = GDK_DRAWABLE_CLASS (klass);

  GDK_GL_NOTE (FUNC, g_message (" - gdk_gl_window_class_init ()"));

  parent_class = g_type_class_peek_parent (klass);

  object_class->set_property = gdk_gl_window_set_property;
  object_class->get_property = gdk_gl_window_get_property;
  object_class->finalize     = gdk_gl_window_finalize;

  drawable_class->create_gc              = gdk_gl_window_create_gc;
  drawable_class->draw_rectangle         = gdk_gl_window_draw_rectangle;
  drawable_class->draw_arc               = gdk_gl_window_draw_arc;
  drawable_class->draw_polygon           = gdk_gl_window_draw_polygon;
  drawable_class->draw_text              = gdk_gl_window_draw_text;
  drawable_class->draw_text_wc           = gdk_gl_window_draw_text_wc;
  drawable_class->draw_drawable          = gdk_gl_window_draw_drawable;
  drawable_class->draw_points            = gdk_gl_window_draw_points;
  drawable_class->draw_segments          = gdk_gl_window_draw_segments;
  drawable_class->draw_lines             = gdk_gl_window_draw_lines;
  drawable_class->draw_glyphs            = gdk_gl_window_draw_glyphs;
  drawable_class->draw_image             = gdk_gl_window_draw_image;
  drawable_class->get_depth              = gdk_gl_window_get_depth;
  drawable_class->get_size               = gdk_gl_window_get_size;
  drawable_class->set_colormap           = gdk_gl_window_set_colormap;
  drawable_class->get_colormap           = gdk_gl_window_get_colormap;
  drawable_class->get_visual             = gdk_gl_window_get_visual;
  drawable_class->get_image              = gdk_gl_window_get_image;
  drawable_class->get_clip_region        = gdk_gl_window_get_clip_region;
  drawable_class->get_visible_region     = gdk_gl_window_get_visible_region;
  drawable_class->get_composite_drawable = gdk_gl_window_get_composite_drawable;
  drawable_class->_draw_pixbuf           = gdk_gl_window_draw_pixbuf;
  drawable_class->_copy_to_image         = gdk_gl_window_copy_to_image;

  g_object_class_install_property (object_class,
                                   PROP_GLCONFIG,
                                   g_param_spec_object ("glconfig",
                                                        _("GL configuration"),
                                                        _("The OpenGL configuration object."),
                                                        GDK_TYPE_GL_CONFIG,
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property (object_class,
                                   PROP_WRAPPER,
                                   g_param_spec_object ("wrapper",
                                                        _("Wrapper"),
                                                        _("Real GdkDrawable object."),
                                                        GDK_TYPE_DRAWABLE,
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

}

static void
gdk_gl_window_set_property (GObject      *object,
                            guint         property_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  GdkGLWindow *glwindow = GDK_GL_WINDOW (object);

  GDK_GL_NOTE (FUNC, g_message (" - gdk_gl_window_set_property ()"));

  switch (property_id)
    {
    case PROP_GLCONFIG:
      glwindow->glconfig = g_value_get_object (value);
      g_object_ref (G_OBJECT (glwindow->glconfig));
      g_object_notify (object, "glconfig");
      break;
    case PROP_WRAPPER:
      glwindow->wrapper = g_value_get_object (value);
      /* g_object_ref (G_OBJECT (glwindow->wrapper)); */
      g_object_notify (object, "wrapper");
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
gdk_gl_window_get_property (GObject    *object,
                            guint       property_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
gdk_gl_window_finalize (GObject *object)
{
  GdkGLWindow *glwindow = GDK_GL_WINDOW (object);

  GDK_GL_NOTE (FUNC, g_message (" - gdk_gl_window_finalize ()"));

  if (glwindow->glconfig != NULL)
    {
      g_object_unref (G_OBJECT (glwindow->glconfig));
      glwindow->glconfig = NULL;
    }

  /*
  if (glwindow->wrapper != NULL)
    {
      g_object_unref (G_OBJECT (glwindow->wrapper));
      glwindow->wrapper = NULL;
    }
  */

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static GdkGC *
gdk_gl_window_create_gc (GdkDrawable    *drawable,
                         GdkGCValues    *values,
                         GdkGCValuesMask mask)
{
  GdkDrawable *real_drawable = ((GdkGLWindow *) drawable)->wrapper;

  return GDK_DRAWABLE_GET_CLASS (real_drawable)->create_gc (real_drawable,
                                                            values,
                                                            mask);
}

static void
gdk_gl_window_draw_rectangle (GdkDrawable *drawable,
                              GdkGC	  *gc,
                              gint	   filled,
                              gint	   x,
                              gint	   y,
                              gint	   width,
                              gint	   height)
{
  GdkDrawable *real_drawable = ((GdkGLWindow *) drawable)->wrapper;

  GDK_DRAWABLE_GET_CLASS (real_drawable)->draw_rectangle (real_drawable,
                                                          gc,
                                                          filled,
                                                          x,
                                                          y,
                                                          width,
                                                          height);
}

static void
gdk_gl_window_draw_arc (GdkDrawable *drawable,
                        GdkGC	    *gc,
                        gint	     filled,
                        gint	     x,
                        gint	     y,
                        gint	     width,
                        gint	     height,
                        gint	     angle1,
                        gint	     angle2)
{
  GdkDrawable *real_drawable = ((GdkGLWindow *) drawable)->wrapper;

  GDK_DRAWABLE_GET_CLASS (real_drawable)->draw_arc (real_drawable,
                                                    gc,
                                                    filled,
                                                    x,
                                                    y,
                                                    width,
                                                    height,
                                                    angle1,
                                                    angle2);
}

static void
gdk_gl_window_draw_polygon (GdkDrawable *drawable,
                            GdkGC	*gc,
                            gint	 filled,
                            GdkPoint    *points,
                            gint	 npoints)
{
  GdkDrawable *real_drawable = ((GdkGLWindow *) drawable)->wrapper;

  GDK_DRAWABLE_GET_CLASS (real_drawable)->draw_polygon (real_drawable,
                                                        gc,
                                                        filled,
                                                        points,
                                                        npoints);
}

static void
gdk_gl_window_draw_text (GdkDrawable *drawable,
                         GdkFont     *font,
                         GdkGC	     *gc,
                         gint	      x,
                         gint	      y,
                         const gchar *text,
                         gint	      text_length)
{
  GdkDrawable *real_drawable = ((GdkGLWindow *) drawable)->wrapper;

  GDK_DRAWABLE_GET_CLASS (real_drawable)->draw_text (real_drawable,
                                                     font,
                                                     gc,
                                                     x,
                                                     y,
                                                     text,
                                                     text_length);
}

static void
gdk_gl_window_draw_text_wc (GdkDrawable	   *drawable,
                            GdkFont	   *font,
                            GdkGC	   *gc,
                            gint	    x,
                            gint            y,
                            const GdkWChar *text,
                            gint	    text_length)
{
  GdkDrawable *real_drawable = ((GdkGLWindow *) drawable)->wrapper;

  GDK_DRAWABLE_GET_CLASS (real_drawable)->draw_text_wc (real_drawable,
                                                        font,
                                                        gc,
                                                        x,
                                                        y,
                                                        text,
                                                        text_length);
}

static void
gdk_gl_window_draw_drawable (GdkDrawable *drawable,
                             GdkGC	 *gc,
                             GdkDrawable *src,
                             gint	  xsrc,
                             gint	  ysrc,
                             gint	  xdest,
                             gint	  ydest,
                             gint	  width,
                             gint	  height)
{
  GdkDrawable *real_drawable = ((GdkGLWindow *) drawable)->wrapper;

  GDK_DRAWABLE_GET_CLASS (real_drawable)->draw_drawable (real_drawable,
                                                         gc,
                                                         src,
                                                         xsrc,
                                                         ysrc,
                                                         xdest,
                                                         ydest,
                                                         width,
                                                         height);
}

static void
gdk_gl_window_draw_points (GdkDrawable *drawable,
                           GdkGC       *gc,
                           GdkPoint    *points,
                           gint	        npoints)
{
  GdkDrawable *real_drawable = ((GdkGLWindow *) drawable)->wrapper;

  GDK_DRAWABLE_GET_CLASS (real_drawable)->draw_points (real_drawable,
                                                       gc,
                                                       points,
                                                       npoints);
}

static void
gdk_gl_window_draw_segments (GdkDrawable *drawable,
                             GdkGC	 *gc,
                             GdkSegment  *segs,
                             gint         nsegs)
{
  GdkDrawable *real_drawable = ((GdkGLWindow *) drawable)->wrapper;

  GDK_DRAWABLE_GET_CLASS (real_drawable)->draw_segments (real_drawable,
                                                         gc,
                                                         segs,
                                                         nsegs);
}

static void
gdk_gl_window_draw_lines (GdkDrawable *drawable,
                          GdkGC       *gc,
                          GdkPoint    *points,
                          gint         npoints)
{
  GdkDrawable *real_drawable = ((GdkGLWindow *) drawable)->wrapper;

  GDK_DRAWABLE_GET_CLASS (real_drawable)->draw_lines (real_drawable,
                                                      gc,
                                                      points,
                                                      npoints);
}

static void
gdk_gl_window_draw_glyphs (GdkDrawable      *drawable,
                           GdkGC            *gc,
                           PangoFont        *font,
                           gint              x,
                           gint              y,
                           PangoGlyphString *glyphs)
{
  GdkDrawable *real_drawable = ((GdkGLWindow *) drawable)->wrapper;

  GDK_DRAWABLE_GET_CLASS (real_drawable)->draw_glyphs (real_drawable,
                                                       gc,
                                                       font,
                                                       x,
                                                       y,
                                                       glyphs);
}

static void
gdk_gl_window_draw_image (GdkDrawable *drawable,
                          GdkGC	      *gc,
                          GdkImage    *image,
                          gint	       xsrc,
                          gint	       ysrc,
                          gint	       xdest,
                          gint	       ydest,
                          gint	       width,
                          gint	       height)
{
  GdkDrawable *real_drawable = ((GdkGLWindow *) drawable)->wrapper;

  GDK_DRAWABLE_GET_CLASS (real_drawable)->draw_image (real_drawable,
                                                      gc,
                                                      image,
                                                      xsrc,
                                                      ysrc,
                                                      xdest,
                                                      ydest,
                                                      width,
                                                      height);
}

static gint
gdk_gl_window_get_depth (GdkDrawable *drawable)
{
  GdkDrawable *real_drawable = ((GdkGLWindow *) drawable)->wrapper;

  return GDK_DRAWABLE_GET_CLASS (real_drawable)->get_depth (real_drawable);
}

static void
gdk_gl_window_get_size (GdkDrawable *drawable,
                        gint        *width,
                        gint        *height)
{
  GdkDrawable *real_drawable = ((GdkGLWindow *) drawable)->wrapper;

  GDK_DRAWABLE_GET_CLASS (real_drawable)->get_size (real_drawable,
                                                    width,
                                                    height);
}

static void
gdk_gl_window_set_colormap (GdkDrawable *drawable,
                            GdkColormap *cmap)
{
  GdkDrawable *real_drawable = ((GdkGLWindow *) drawable)->wrapper;

  GDK_DRAWABLE_GET_CLASS (real_drawable)->set_colormap (real_drawable,
                                                        cmap);
}

static GdkColormap *
gdk_gl_window_get_colormap (GdkDrawable *drawable)
{
  GdkDrawable *real_drawable = ((GdkGLWindow *) drawable)->wrapper;

  return GDK_DRAWABLE_GET_CLASS (real_drawable)->get_colormap (real_drawable);
}

static GdkVisual *
gdk_gl_window_get_visual (GdkDrawable *drawable)
{
  GdkDrawable *real_drawable = ((GdkGLWindow *) drawable)->wrapper;

  return GDK_DRAWABLE_GET_CLASS (real_drawable)->get_visual (real_drawable);
}

static GdkImage *
gdk_gl_window_get_image (GdkDrawable *drawable,
                         gint         x,
                         gint         y,
                         gint         width,
                         gint         height)
{
  GdkDrawable *real_drawable = ((GdkGLWindow *) drawable)->wrapper;

  return GDK_DRAWABLE_GET_CLASS (real_drawable)->get_image (real_drawable,
                                                            x,
                                                            y,
                                                            width,
                                                            height);
}

static GdkRegion *
gdk_gl_window_get_clip_region (GdkDrawable *drawable)
{
  GdkDrawable *real_drawable = ((GdkGLWindow *) drawable)->wrapper;

  return GDK_DRAWABLE_GET_CLASS (real_drawable)->get_clip_region (real_drawable);
}

static GdkRegion *
gdk_gl_window_get_visible_region (GdkDrawable *drawable)
{
  GdkDrawable *real_drawable = ((GdkGLWindow *) drawable)->wrapper;

  return GDK_DRAWABLE_GET_CLASS (real_drawable)->get_visible_region (real_drawable);
}

static GdkDrawable *
gdk_gl_window_get_composite_drawable (GdkDrawable *drawable,
                                      gint         x,
                                      gint         y,
                                      gint         width,
                                      gint         height,
                                      gint        *composite_x_offset,
                                      gint        *composite_y_offset)
{
  GdkDrawable *real_drawable = ((GdkGLWindow *) drawable)->wrapper;

  return GDK_DRAWABLE_GET_CLASS (real_drawable)->get_composite_drawable (real_drawable,
                                                                         x,
                                                                         y,
                                                                         width,
                                                                         height,
                                                                         composite_x_offset,
                                                                         composite_y_offset);
}

static void
gdk_gl_window_draw_pixbuf (GdkDrawable *drawable,
                           GdkGC       *gc,
                           GdkPixbuf   *pixbuf,
                           gint         src_x,
                           gint         src_y,
                           gint         dest_x,
                           gint         dest_y,
                           gint         width,
                           gint         height,
                           GdkRgbDither dither,
                           gint         x_dither,
                           gint         y_dither)
{
  GdkDrawable *real_drawable = ((GdkGLWindow *) drawable)->wrapper;

  GDK_DRAWABLE_GET_CLASS (real_drawable)->_draw_pixbuf (real_drawable,
                                                        gc,
                                                        pixbuf,
                                                        src_x,
                                                        src_y,
                                                        dest_x,
                                                        dest_y,
                                                        width,
                                                        height,
                                                        dither,
                                                        x_dither,
                                                        y_dither);
}

static GdkImage *
gdk_gl_window_copy_to_image (GdkDrawable *drawable,
                             GdkImage    *image,
                             gint         src_x,
                             gint         src_y,
                             gint         dest_x,
                             gint         dest_y,
                             gint         width,
                             gint         height)
{
  GdkDrawable *real_drawable = ((GdkGLWindow *) drawable)->wrapper;

  return GDK_DRAWABLE_GET_CLASS (real_drawable)->_copy_to_image (real_drawable,
                                                                 image,
                                                                 src_x,
                                                                 src_y,
                                                                 dest_x,
                                                                 dest_y,
                                                                 width,
                                                                 height);
}

/*< private >*/
gboolean
_gdk_gl_window_is_double_buffered (GdkGLDrawable *gldrawable)
{
  g_return_val_if_fail (GDK_IS_GL_WINDOW (gldrawable), FALSE);

  return gdk_gl_config_is_double_buffered (GDK_GL_WINDOW (gldrawable)->glconfig);
}

/*< private >*/
GdkGLConfig *
_gdk_gl_window_get_gl_config (GdkGLDrawable *gldrawable)
{
  g_return_val_if_fail (GDK_IS_GL_WINDOW (gldrawable), FALSE);

  return GDK_GL_WINDOW (gldrawable)->glconfig;
}

/*
 * OpenGL extension to GdkWindow
 */

static const gchar quark_gl_window_string[] = "gdk-gl-window-gl-window";
static GQuark quark_gl_window = 0;

static void
gl_window_destroy (GdkGLWindow *glwindow)
{
  if (glwindow != NULL)
    g_object_unref (G_OBJECT (glwindow));
}

/**
 * gdk_window_set_gl_capability:
 * @window: the #GdkWindow to be used as the rendering area.
 * @glconfig: a #GdkGLConfig.
 * @attrib_list: this must be set to NULL or empty (first attribute of None).
 *
 * Set the OpenGL-capability to the @window.
 * The call creates a new #GdkGLWindow.
 * attrib_list is currently unused. This must be set to NULL or empty
 * (first attribute of None).
 *
 * Return value: the #GdkGLWindow used by the @window if it is successful, NULL otherwise.
 **/
GdkGLWindow *
gdk_window_set_gl_capability (GdkWindow    *window,
                              GdkGLConfig  *glconfig,
                              const gint   *attrib_list)
{
  GdkGLWindow *glwindow;

  g_return_val_if_fail (GDK_IS_WINDOW (window), FALSE);
  g_return_val_if_fail (GDK_IS_GL_CONFIG (glconfig), FALSE);

  /*
   * Initialize quarks
   */

  if (quark_gl_window == 0)
    quark_gl_window = g_quark_from_static_string (quark_gl_window_string);

  /* If already set */
  glwindow = g_object_get_qdata (G_OBJECT (window), quark_gl_window);
  if (glwindow != NULL)
    return glwindow;

  /*
   * Create GdkGLWindow
   */

  glwindow = gdk_gl_window_new (glconfig, window, attrib_list);
  if (glwindow == NULL)
    {
      g_warning ("cannot create GdkGLWindow\n");
      goto FAIL;
    }

  g_object_set_qdata_full (G_OBJECT (window), quark_gl_window, glwindow,
                           (GDestroyNotify) gl_window_destroy);

  return glwindow;

 FAIL:

  g_object_set_qdata (G_OBJECT (window), quark_gl_window, NULL);

  return NULL;
}

/**
 * gdk_window_unset_gl_capability:
 * @window: a #GdkWindow.
 *
 * Unset the OpenGL-capability of the @window.
 * The call destroys the #GdkGLWindow holded by the @window.
 *
 **/
void
gdk_window_unset_gl_capability (GdkWindow *window)
{
  /*
   * If quarks are not initialized
   */

  if (quark_gl_window == 0)
    quark_gl_window = g_quark_from_static_string (quark_gl_window_string);

  /*
   * Unref OpenGL-related data of the window
   */

  g_object_set_qdata (G_OBJECT (window), quark_gl_window, NULL);
}

/**
 * gdk_window_is_gl_capable:
 * @window: a #GdkWindow.
 *
 * Return whether the @window is OpenGL-capable.
 *
 * Return value: TRUE if the @window is OpenGL-capable, FALSE otherwise.
 **/
gboolean
gdk_window_is_gl_capable (GdkWindow *window)
{
  g_return_val_if_fail (GDK_IS_WINDOW (window), FALSE);

  return g_object_get_qdata (G_OBJECT (window), quark_gl_window) != NULL ? TRUE : FALSE;
}

/**
 * gdk_window_get_gl_window:
 * @window: a #GdkWindow.
 *
 * Return the #GdkGLWindow holded by the @window.
 *
 * Return value: the #GdkGLWindow.
 **/
GdkGLWindow *
gdk_window_get_gl_window (GdkWindow *window)
{
  g_return_val_if_fail (GDK_IS_WINDOW (window), NULL);

  return g_object_get_qdata (G_OBJECT (window), quark_gl_window);
}
