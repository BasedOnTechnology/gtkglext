/* GdkGLExt - OpenGL Extension to GDK
 * Copyright (C) 2002-2004  Naofumi Yasufuku
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

#if !defined (__GDKGLWIN32_H_INSIDE__) && !defined (GDK_GL_COMPILATION)
#error "Only <gdk/gdkglwin32.h> can be included directly."
#endif

#ifndef __GDK_WIN32_GL_CONFIG_H__
#define __GDK_WIN32_GL_CONFIG_H__

#include <gdk/gdkwin32.h>
#include <gdk/gdkgl.h>

G_BEGIN_DECLS

GdkGLConfig           *gdk_win32_gl_config_new_from_pixel_format (int pixel_format);

PIXELFORMATDESCRIPTOR *gdk_win32_gl_config_get_pfd               (GdkGLConfig   *glconfig);

G_END_DECLS

#ifdef INSIDE_GDK_GL_WIN32
#define GDK_GL_CONFIG_PFD(glconfig)          (&(GDK_GL_CONFIG_IMPL_WIN32 (glconfig)->pfd))
#else
#define GDK_GL_CONFIG_PFD(glconfig)          (gdk_win32_gl_config_get_pfd (glconfig))
#endif

#endif /* __GDK_WIN32_GL_CONFIG_H__ */
