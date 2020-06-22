glib_dep = dependency('glib-2.0')
gobject_dep = dependency('gobject-2.0')
gtk_dep = dependency('gtk+-3.0')

executable('kiran-calendar-test',
  'kiran-calendar-test.c',
  install : false,
  c_args: '-g',
  c_args: '-DWNCK_I_KNOW_THIS_IS_UNSTABLE',
  dependencies : [glib_dep, gobject_dep, gtk_dep]
)
