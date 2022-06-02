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

/*
 * kiran caleandar 是一个可以显示中国农历的日历组件
 */

#ifndef __KIRAN_CALENDAR_H__
#define __KIRAN_CALENDAR_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define CALENDA_WIDTH 304
#define CALENDA_HEIGHT 495

#define KIRAN_TYPE_CALENDAR (kiran_calendar_get_type())
#define KIRAN_CALENDAR(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), KIRAN_TYPE_CALENDAR, KiranCalendar))
#define KIRAN_CALENDAR_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), KIRAN_TYPE_CALENDAR, KiranCalendarClass))
#define KIRAN_IS_CALENDAR(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), KIRAN_TYPE_CALENDAR))
#define KIRAN_IS_CALENDAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), KIRAN_TYPE_CALENDAR))
#define KIRAN_CALENDAR_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), KIRAN_TYPE_CALENDAR, KiranCalendarClass))

typedef struct _KiranCalendar KiranCalendar;
typedef struct _KiranCalendarClass KiranCalendarClass;

typedef struct _KiranCalendarPrivate KiranCalendarPrivate;

struct _KiranCalendar
{
    GtkWidget parent;

    KiranCalendarPrivate *priv;
};

struct _KiranCalendarClass
{
    GtkWidgetClass parent_class;
};

GType kiran_calendar_get_type(void);
GtkWidget *kiran_calendar_new(void);
void kiran_calendar_select_day(KiranCalendar *calendar,
                               guint day);
void kiran_calendar_refresh(KiranCalendar *calendar);

G_END_DECLS

#endif /* __KIRAN_CALENDAR_H__ */
