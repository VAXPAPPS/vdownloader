project('vxap-downloader', 'c',
  version: '0.1.0',
  meson_version: '>= 0.60.0',
  default_options: [
    'c_std=c17',
    'warning_level=2',
  ]
)

cc = meson.get_compiler('c')

# Dependencies
gtk4_dep = dependency('gtk4', version: '>= 4.6')
adw_dep  = dependency('libadwaita-1', version: '>= 1.2', required: false)
curl_dep = dependency('libcurl', version: '>= 7.68')
json_dep = dependency('json-glib-1.0', version: '>= 1.6')
notify_dep = dependency('libnotify', version: '>= 0.7', required: false)

deps = [gtk4_dep, curl_dep, json_dep]

if adw_dep.found()
  deps += [adw_dep]
  add_project_arguments('-DHAVE_ADWAITA', language: 'c')
endif

if notify_dep.found()
  deps += [notify_dep]
  add_project_arguments('-DHAVE_LIBNOTIFY', language: 'c')
endif

add_project_arguments('-DAPP_ID="com.vxap.downloader"', language: 'c')
add_project_arguments('-DAPP_VERSION="' + meson.project_version() + '"', language: 'c')

# GResources
gnome = import('gnome')
resources = gnome.compile_resources(
  'vxap-resources',
  'resources/vxap.gresource.xml',
  source_dir: 'resources',
  c_name: 'vxap'
)

# Source files
sources = files(
  'src/main.c',
  'src/app/vxap_app.c',
  'src/app/vxap_config.c',
  'src/ui/window/main_window.c',
  'src/ui/window/add_dialog.c',
  'src/ui/widgets/download_row.c',
  'src/ui/widgets/sidebar.c',
  'src/ui/widgets/statusbar.c',
  'src/aria2/aria2_client.c',
  'src/aria2/aria2_types.c',
  'src/aria2/aria2_monitor.c',
  'src/core/download_manager.c',
  'src/core/download_queue.c',
  'src/core/utils.c',
  'src/ui/theme_manager.c',
)

include_dirs = include_directories('src')

executable('vxap-downloader',
  sources,
  resources,
  include_directories: include_dirs,
  dependencies: deps,
  install: true,
)

# GSettings schema
install_data('data/com.vxap.downloader.gschema.xml',
  install_dir: get_option('datadir') / 'glib-2.0' / 'schemas'
)

# Desktop file
install_data('data/com.vxap.downloader.desktop',
  install_dir: get_option('datadir') / 'applications'
)

# Icon file
install_data('vdownloader.svg',
  rename: 'com.vxap.downloader.svg',
  install_dir: get_option('datadir') / 'icons' / 'hicolor' / 'scalable' / 'apps'
)
