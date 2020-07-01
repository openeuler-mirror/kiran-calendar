project ('kiran-calendar', 'c',
  version : '1.22',
  license : 'LGPLv2+',
  meson_version : '>=0.44',
  default_options : [
    'c_std=gnu99',
    'prefix=/usr',
    'sysconfdir=/etc'
  ]
)

compiler = meson.get_compiler('c')
project_name = meson.project_name()
project_version = meson.project_version()

prefix = get_option('prefix')
datadir = join_paths(prefix, get_option('datadir'))

i18n  = import('i18n')

subdir('data')
subdir('src')
subdir('po')
