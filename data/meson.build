appletconf = configuration_data()
appletconf.set('VERSION', meson.project_version())
appletconf.set('IN_PROCESS', 'false')
appletconf.set('libexecdir', libexecdir)
appleatfile = 'org.mate.panel.KiranClockApplet.mate-panel-applet'

appleatdir = join_paths (datadir, panel_name, 'applets')
configure_file(
  input: appleatfile + '.in.in',
  output: appleatfile,
  configuration: appletconf,
  install: true,
  install_dir: appleatdir 
)


serviceconf = configuration_data()
serviceconf.set('libexecdir', libexecdir)

if (get_option('kiran_desktop'))
  sevicefile = 'org.kiran.panel.applet.KiranClockAppletFactory.service'
else
  sevicefile = 'org.mate.panel.applet.KiranClockAppletFactory.service'
endif

serivedir = join_paths(datadir, 'dbus-1', 'services')
configure_file(
  input: sevicefile + '.in',
  output: sevicefile,
  configuration: serviceconf,
  install: true,
  install_dir: serivedir
)

schemaconf = configuration_data()
configure_file (
    input: 'com.kylinsec.kiran.calendar.gschema.xml.in',
    output: 'com.kylinsec.kiran.calendar.gschema.xml',
    configuration: schemaconf,
    install_dir: schemadir
)

filedir = join_paths(datadir, project_name)
install_data('kiran-calendar.css', install_dir: filedir)
install_data('setting_btn.svg', install_dir: filedir)
