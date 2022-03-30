/**
* @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; If not, see <http: //www.gnu.org/licenses/>. 
*
* Author:     wangxiaoqing <wangxiaoqing@kylinos.com.cn>
*/

/**
 * kiran calendar window 
 *
 */

#ifndef __KIRAN_CALENDAR_WINDOW_H__
#define __KIRAN_CALENDAR_WINDOW_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define KIRAN_TYPE_CALENDAR_WINDOW (kiran_calendar_window_get_type())
#define KIRAN_CALENDAR_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), KIRAN_TYPE_CALENDAR_WINDOW, KiranCalendarWindow))
#define KIRAN_CALENDAR_WINDOW_CLASS(klass) (G_TYPPE_CHECK_CLASS_CAST((klass), KIRAN_TYPE_CALENDAR_WINDOW, kiranCalendarWindowClass))
#define KIRAN_IS_CALENDAR_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), KIRAN_TYPE_CALENDAR_WINDOW))
#define KIRAN_IS_CALENDAR_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), KIRAN_TYPE_CALENDAR_WINDOW))
#define KIRAN_CALENDAR_WINDOW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), KIRAN_TYPE_CALENDAR_WINDOW, KiranCalendarWindowClass))

typedef struct _KiranCalendarWindow KiranCalendarWindow;
typedef struct _KiranCalendarWindowClass KiranCalendarWindowClass;
typedef struct _KiranCalendarWindowPrivate KiranCalendarWindowPrivate;

struct _KiranCalendarWindow
{
    GtkWindow parent;
    KiranCalendarWindowPrivate *priv;
};

struct _KiranCalendarWindowClass
{
    GtkWindowClass parent_class;
};

GType kiran_calendar_window_get_type(void);
GtkWidget *kiran_calendar_window_new(void);
void kiran_calendar_window_show(KiranCalendarWindow *window);
void kiran_calendar_window_hide(KiranCalendarWindow *window);

G_END_DECLS

#endif /* __KIRAN_CALENDAR_WINDOW_H__ */
