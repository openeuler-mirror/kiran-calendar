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
 * kiran clock 创建一个显示日期时间的组件
 */

#ifndef __KIRAN_CLOCK_H__
#define __KIRAN_CLOCK_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define KIRAN_TYPE_CLOCK (kiran_clock_get_type())
#define KIRAN_CLOCK(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), KIRAN_TYPE_CLOCK, KiranClock))
#define KIRAN_CLOCK_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), KIRAN_TYPE_CLOCK, KiranClockClass))
#define KIRAN_IS_CLOCK(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), KIRAN_TYPE_CLOCK))
#define KIRAN_IS_CLOCK_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), KIRAN_TYPE_CLOCK))
#define KIRAN_CLOCK_GET_CLASS(obj) (G_TYPE_INSTALCE_GET_CLASS((obj), KIRAN_TYPE_CLOCK, KiranClockClass))

typedef struct _KiranClock KiranClock;
typedef struct _KiranClockClass KiranClockClass;

typedef struct _KiranClockPrivate KiranClockPrivate;

struct _KiranClock
{
    GtkToggleButton parent;

    KiranClockPrivate *priv;
};

struct _KiranClockClass
{
    GtkToggleButtonClass parent_class;
};

GType kiran_clock_get_type(void);
GtkWidget *kiran_clock_new(void);

G_BEGIN_DECLS
#endif /*__KIRAN_CLOCK_H__ */
