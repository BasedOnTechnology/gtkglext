/*
 * GL_ARB_multitexture demo
 *
 * Brian Paul  November 1998  This program is in the public domain.
 * Modified on 12 Feb 2002 for > 2 texture units.
 */

/* Conversion to GtkGLExt by Naofumi Yasufuku */

#include <stdlib.h>
#include <math.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include <gtk/gtkgl.h>
#include <gdk/gdkglglext.h>     /* for GL_ARB_multitexture extension */

#include <GL/gl.h>

#include "readtex.h"

#define TEXTURE_1_FILE "girl.rgb"
#define TEXTURE_2_FILE "reflect.rgb"

#define TEX0 1
#define TEX7 8
#define ANIMATE 10
#define QUIT 100

static GLint num_units = 1;
static GLboolean tex_enabled[8];

static GLfloat drift = 0.0;
static GLfloat x_rot = 20.0, y_rot = 30.0, z_rot = 0.0;

static gboolean animate = TRUE;

/* GL_ARB_multitexture extension */
static GdkGL_GL_ARB_multitexture *gl_arb_mt = NULL;

static void
init (GtkWidget *widget,
      gpointer   data)
{
  GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
  GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);

  GLuint texObj[8];
  GLint size, i;

  /*** OpenGL BEGIN ***/
  if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext))
    return;

  /*
   * Get GL_ARB_multitexture extension functions.
   */
  gl_arb_mt = gdk_gl_get_GL_ARB_multitexture ();
  if (gl_arb_mt == NULL)
    {
      g_print ("Sorry, GL_ARB_multitexture not supported by this renderer.\n");
      gtk_main_quit ();
    }

  glGetIntegerv (GL_MAX_TEXTURE_UNITS_ARB, &num_units);
  g_print ("%d texture units supported\n", num_units);
  if (num_units > 8)
    num_units = 8;

  glGetIntegerv (GL_MAX_TEXTURE_SIZE, &size);
  g_print ("%d x %d max texture size\n", size, size);

  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);

  for (i = 0; i < num_units; i++)
    {
      if (i < 2)
	tex_enabled[i] = GL_TRUE;
      else
	tex_enabled[i] = GL_FALSE;
    }

  /* allocate two texture objects */
  glGenTextures (num_units, texObj);

  /* setup the texture objects */
  for (i = 0; i < num_units; i++)
    {

      gl_arb_mt->glActiveTextureARB (GL_TEXTURE0_ARB + i);
      glBindTexture (GL_TEXTURE_2D, texObj[i]);

      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

      if (i == 0)
	{
	  if (!LoadRGBMipmaps (TEXTURE_1_FILE, GL_RGB))
	    {
	      g_print ("Error: couldn't load texture image\n");
	      gtk_main_quit ();
	    }
	}
      else if (i == 1)
	{
	  if (!LoadRGBMipmaps (TEXTURE_2_FILE, GL_RGB))
	    {
	      g_print ("Error: couldn't load texture image\n");
	      gtk_main_quit ();
	    }
	}
      else
	{
	  /* checker */
	  GLubyte image[8][8][3];
	  GLint i, j;
	  for (i = 0; i < 8; i++)
	    {
	      for (j = 0; j < 8; j++)
		{
		  if ((i + j) & 1)
		    {
		      image[i][j][0] = 50;
		      image[i][j][1] = 50;
		      image[i][j][2] = 50;
		    }
		  else
		    {
		      image[i][j][0] = 25;
		      image[i][j][1] = 25;
		      image[i][j][2] = 25;
		    }
		}
	    }
	  glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, 8, 8, 0,
			GL_RGB, GL_UNSIGNED_BYTE, (GLvoid *) image);
	}

      /* Bind texObj[i] to ith texture unit */
      if (i < 2)
	glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      else
	glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);

      if (tex_enabled[i])
	glEnable (GL_TEXTURE_2D);
    }

  glShadeModel (GL_FLAT);
  glClearColor (0.3, 0.3, 0.4, 1.0);

  g_print ("\n");
  g_print ("GL_RENDERER   = %s\n", (char *) glGetString (GL_RENDERER));
  g_print ("GL_VERSION    = %s\n", (char *) glGetString (GL_VERSION));
  g_print ("GL_VENDOR     = %s\n", (char *) glGetString (GL_VENDOR));
  g_print ("GL_EXTENSIONS = %s\n", (char *) glGetString (GL_EXTENSIONS));
  g_print ("\n");

  gdk_gl_drawable_gl_end (gldrawable);
  /*** OpenGL END ***/
}

static gboolean
reshape (GtkWidget         *widget,
	 GdkEventConfigure *event,
	 gpointer           data)
{
  GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
  GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);

  /*** OpenGL BEGIN ***/
  if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext))
    return FALSE;

  glViewport (0, 0, widget->allocation.width, widget->allocation.height);
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  glFrustum (-1.0, 1.0, -1.0, 1.0, 10.0, 100.0);
  /* glOrtho (-6.0, 6.0, -6.0, 6.0, 10.0, 100.0); */
  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();
  glTranslatef (0.0, 0.0, -70.0);

  gdk_gl_drawable_gl_end (gldrawable);
  /*** OpenGL END ***/

  return TRUE;
}

static void
draw_object (void)
{
  GLint i;

  if (!tex_enabled[0] && !tex_enabled[1])
    glColor3f (0.1, 0.1, 0.1);  /* add onto this */
  else
    glColor3f (1, 1, 1);  /* modulate this */

  glBegin (GL_QUADS);

  for (i = 0; i < num_units; i++)
    gl_arb_mt->glMultiTexCoord2fARB (GL_TEXTURE0_ARB + i, 0.0, 0.0);
  glVertex2f (-1.0, -1.0);

  for (i = 0; i < num_units; i++)
    gl_arb_mt->glMultiTexCoord2fARB (GL_TEXTURE0_ARB + i, 1.0, 0.0);
  glVertex2f (1.0, -1.0);

  for (i = 0; i < num_units; i++)
    gl_arb_mt->glMultiTexCoord2fARB (GL_TEXTURE0_ARB + i, 1.0, 1.0);
  glVertex2f (1.0, 1.0);

  for (i = 0; i < num_units; i++)
    gl_arb_mt->glMultiTexCoord2fARB (GL_TEXTURE0_ARB + i, 0.0, 1.0);
  glVertex2f (-1.0, 1.0);

  glEnd ();
}

static gboolean
display (GtkWidget      *widget,
	 GdkEventExpose *event,
	 gpointer        data)
{
  GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
  GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);

  /*** OpenGL BEGIN ***/
  if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext))
    return FALSE;

  glClear (GL_COLOR_BUFFER_BIT);

  glPushMatrix ();
    glRotatef (x_rot, 1.0, 0.0, 0.0);
    glRotatef (y_rot, 0.0, 1.0, 0.0);
    glRotatef (z_rot, 0.0, 0.0, 1.0);
    glScalef (5.0, 5.0, 5.0);
    draw_object ();
  glPopMatrix ();

  if (gdk_gl_drawable_is_double_buffered (gldrawable))
    gdk_gl_drawable_swap_buffers (gldrawable);
  else
    glFlush ();

  gdk_gl_drawable_gl_end (gldrawable);
  /*** OpenGL END ***/

  return TRUE;
}

static gboolean
idle (GtkWidget *widget)
{
  GLint i;

  drift += 0.05;
  if (drift >= 1.0)
    drift = 0.0;

  for (i = 0; i < num_units; i++)
    {
      gl_arb_mt->glActiveTextureARB (GL_TEXTURE0_ARB + i);
      glMatrixMode (GL_TEXTURE);
      glLoadIdentity ();
      if (i == 0)
	{
	  glTranslatef (drift, 0.0, 0.0);
	  glScalef (2, 2, 2);
	}
      else if (i == 1)
	{
	  glTranslatef (0.0, drift, 0.0);
	}
      else
	{
	  glTranslatef (0.5, 0.5, 0.0);
	  glRotatef (180.0 * drift, 0, 0, 1);
	  glScalef (1.0/i, 1.0/i, 1.0/i);
	  glTranslatef (-0.5, -0.5, 0.0);
	}
    }
  glMatrixMode (GL_MODELVIEW);

  gtk_widget_queue_draw (widget);

  return TRUE;
}

static guint idle_id = 0;

static void
idle_add (GtkWidget *widget)
{
  if (idle_id == 0)
    {
      idle_id = gtk_idle_add_priority (GDK_PRIORITY_REDRAW,
				       (GtkFunction) idle,
				       widget);
    }
}

static void
idle_remove (GtkWidget *widget)
{
  if (idle_id != 0)
    {
      gtk_idle_remove (idle_id);
      idle_id = 0;
    }
}

static gboolean
map (GtkWidget   *widget,
     GdkEventAny *event,
     gpointer     data)
{
  if (animate)
    idle_add (widget);

  return TRUE;
}

static gboolean
unmap (GtkWidget   *widget,
       GdkEventAny *event,
       gpointer     data)
{
  idle_remove (widget);

  return TRUE;
}

static gboolean
visible (GtkWidget          *widget,
	 GdkEventVisibility *event,
	 gpointer            data)
{
  if (animate)
    {
      if (event->state == GDK_VISIBILITY_FULLY_OBSCURED)
	idle_remove (widget);
      else
	idle_add (widget);
    }

  return TRUE;
}

static void
toggle_animation (GtkWidget *widget)
{
  animate = !animate;

  if (animate)
    idle_add (widget);
  else
    idle_remove (widget);
}

static gboolean
button_press (GtkWidget      *widget,
	      GdkEventButton *event,
	      GtkWidget      *menu)
{
  /* Popup menu. */
  if (event->button == 3)
    gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL,
		    event->button, event->time);

  return TRUE;
}

/* change view angle, exit upon ESC */
static gboolean
key (GtkWidget   *widget,
     GdkEventKey *event,
     gpointer     data)
{
  float step = 3.0;

  switch (event->keyval)
    {
    case GDK_Up:
      x_rot += step;
      break;
    case GDK_Down:
      x_rot -= step;
      break;
    case GDK_Left:
      y_rot += step;
      break;
    case GDK_Right:
      y_rot -= step;
      break;
    case GDK_a:
      toggle_animation (widget);
      break;
    case GDK_Escape:
      gtk_main_quit ();
      break;
    default:
      return TRUE;
    }

  if (!animate)
    gtk_widget_queue_draw (widget);

  return TRUE;
}

int
main (int   argc,
      char *argv[])
{
  GdkGLConfig *glconfig;
  GtkWidget *window;
  GtkWidget *vbox;
  GtkWidget *drawing_area;
  GtkWidget *menu, *menu_item;

  /*
   * Init GTK.
   */

  gtk_init (&argc, &argv);

  /*
   * Init GtkGLExt.
   */

  gtk_gl_init (&argc, &argv);

  /*
   * Configure OpenGL-capable visual.
   */

  /* Try double-buffered visual */
  glconfig = gdk_gl_config_new_by_mode (GDK_GL_MODE_RGB |
					GDK_GL_MODE_DOUBLE);
  if  (glconfig == NULL)
    {
      g_print ("*** Cannot find the double-buffered visual.\n");
      g_print ("*** Trying single-buffered visual.\n");

      /* Try single-buffered visual */
      glconfig = gdk_gl_config_new_by_mode (GDK_GL_MODE_RGB);
      if  (glconfig == NULL)
	{
	  g_print ("*** No appropriate OpenGL-capable visual found.\n");
	  exit (1);
	}
    }

  /*
   * Top-level window.
   */

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "multiarb");

  /*
   * If window manager doesn't watch the WM_COLORMAP_WINDOWS property on
   * a top-level window, we have to set OpenGL window's colormap to the
   * top-level window.
   */
  gtk_widget_set_colormap  (window, gdk_gl_config_get_colormap  (glconfig));

  g_signal_connect (G_OBJECT (window), "delete_event",
		    G_CALLBACK (gtk_main_quit), NULL);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (window), vbox);
  gtk_widget_show (vbox);

  /*
   * Drawing area for drawing OpenGL scene.
   */

  drawing_area = gtk_drawing_area_new ();
  gtk_widget_set_size_request (drawing_area, 300, 300);

  /* Set OpenGL-capability to the widget. */
  gtk_widget_set_gl_capability (drawing_area,
				glconfig,
				NULL,
				TRUE,
				GDK_GL_RGBA_TYPE);

  gtk_box_pack_start (GTK_BOX (vbox), drawing_area, TRUE, TRUE, 0);

  gtk_widget_set_events (drawing_area,
			 GDK_EXPOSURE_MASK |
			 GDK_BUTTON_PRESS_MASK |
			 GDK_VISIBILITY_NOTIFY_MASK);

  g_signal_connect (G_OBJECT (drawing_area), "realize",
		    G_CALLBACK (init), NULL);
  g_signal_connect (G_OBJECT (drawing_area), "configure_event",
		    G_CALLBACK (reshape), NULL);
  g_signal_connect (G_OBJECT (drawing_area), "expose_event",
		    G_CALLBACK (display), NULL);
  g_signal_connect (G_OBJECT (drawing_area), "map_event",
		    G_CALLBACK (map), NULL);
  g_signal_connect (G_OBJECT (drawing_area), "unmap_event",
		    G_CALLBACK (unmap), NULL);
  g_signal_connect (G_OBJECT (drawing_area), "visibility_notify_event",
		    G_CALLBACK (visible), NULL);

  g_signal_connect_swapped (G_OBJECT (window), "key_press_event",
			    G_CALLBACK (key), drawing_area);

  gtk_widget_show (drawing_area);

  /*
   * Popup menu.
   */

  menu = gtk_menu_new ();

  /* Toggle animation */
  menu_item = gtk_menu_item_new_with_label ("Toggle Animation");
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
  g_signal_connect_swapped (G_OBJECT (menu_item), "activate",
			    G_CALLBACK (toggle_animation), drawing_area);
  gtk_widget_show (menu_item);

  /* Quit */
  menu_item = gtk_menu_item_new_with_label ("Quit");
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
  g_signal_connect (G_OBJECT (menu_item), "activate",
		    G_CALLBACK (gtk_main_quit), NULL);
  gtk_widget_show (menu_item);

  /* Signal handler */
  g_signal_connect (G_OBJECT (drawing_area), "button_press_event",
		    G_CALLBACK (button_press), menu);

  /*
   * Show window.
   */

  gtk_widget_show (window);

  /*
   * Main loop.
   */

  gtk_main ();

  return 0;
}
