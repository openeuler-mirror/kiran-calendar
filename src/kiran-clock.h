/*
 * kiran clock 创建一个显示日期时间的组件
 */

#ifndef __KIRAN_CLOCK_H__
#define __KIRAN_CLOCK_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define KIRAN_TYPE_CLOCK 		(kiran_clock_get_type ())
#define KIRAN_CLOCK(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), KIRAN_TYPE_CLOCK, KiranClock))
#define KIRAN_CLOCK_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), KIRAN_TYPE_CLOCK, KiranClockClass))
#define KIRAN_IS_CLOCK(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), KIRAN_TYPE_CLOCK))
#define KIRAN_IS_CLOCK_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), KIRAN_TYPE_CLOCK))
#define KIRAN_CLOCK_GET_CLASS(obj) (G_TYPE_INSTALCE_GET_CLASS ((obj), KIRAN_TYPE_CLOCK, KiranClockClass))

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

GType kiran_clock_get_type 	(void);
GtkWidget *kiran_clock_new 	(void);

G_BEGIN_DECLS
#endif /*__KIRAN_CLOCK_H__ */
