project('vxap-downloader', ['c', 'cpp'],
  version: '1.0.0',
  license: 'GPL-3.0',
  meson_version: '>= 0.59.0',
  default_options: [
    'c_std=gnu17',
    'cpp_std=c++17',
    'warning_level=2',
  ],
)

app_id = 'org.vaxp.downloader'

# Dependencies
gtk4_dep = dependency('gtk4', version: '>= 4.6')
libadwaita_dep = dependency('libadwaita-1', version: '>= 1.2')
libcurl_dep = dependency('libcurl', version: '>= 7.68')
sqlite3_dep = dependency('sqlite3', version: '>= 3.31')
libnotify_dep = dependency('libnotify', version: '>= 0.7')
threads_dep = dependency('threads')
json_glib_dep = dependency('json-glib-1.0')
openssl_dep = dependency('openssl', version: '>= 1.1')
libtorrent_dep = dependency('libtorrent-rasterbar', version: '>= 2.0')

cc = meson.get_compiler('c')
math_dep = cc.find_library('m', required: true)

# GResource compilation
gnome = import('gnome')

resources = gnome.compile_resources(
  'vdl-resources',
  'data/org.vaxp.downloader.gresource.xml',
  source_dir: 'data',
  c_name: 'vdl',
)

# C source files
core_sources = files(
  'src/core/vdl-application.c',
  'src/core/vdl-config.c',
  'src/core/vdl-logger.c',
  'src/core/vdl-utils.c',
)

domain_sources = files(
  'src/domain/entities/vdl-download-item.c',
  'src/domain/entities/vdl-category.c',
)

data_sources = files(
  'src/data/repositories/vdl-sqlite-repo.c',
  'src/data/migrations/vdl-db-migration.c',
)

engine_c_sources = files(
  'src/engine/http/vdl-http-engine.c',
  'src/engine/http/vdl-segment.c',
  'src/engine/http/vdl-chunk-manager.c',
  'src/engine/http/vdl-speed-limiter.c',
  'src/engine/scheduler/vdl-queue-manager.c',
  'src/engine/scheduler/vdl-scheduler.c',
)

# C++ source files (libtorrent wrapper)
engine_cpp_sources = files(
  'src/engine/torrent/vdl-torrent-engine.cpp',
)

services_sources = files(
  'src/services/vdl-clipboard-monitor.c',
  'src/services/vdl-notification-service.c',
  'src/services/vdl-checksum-verifier.c',
)

presentation_sources = files(
  'src/presentation/windows/vdl-main-window.c',
  'src/presentation/views/vdl-downloads-view.c',
  'src/presentation/views/vdl-categories-view.c',
  'src/presentation/views/vdl-statistics-view.c',
  'src/presentation/views/vdl-torrents-view.c',
  'src/presentation/widgets/vdl-download-row.c',
  'src/presentation/widgets/vdl-progress-ring.c',
  'src/presentation/widgets/vdl-speed-graph.c',
  'src/presentation/widgets/vdl-sidebar.c',
  'src/presentation/widgets/vdl-headerbar.c',
  'src/presentation/dialogs/vdl-add-download-dialog.c',
  'src/presentation/dialogs/vdl-preferences-dialog.c',
)

all_sources = [
  'src/main.c',
  core_sources,
  domain_sources,
  data_sources,
  engine_c_sources,
  engine_cpp_sources,
  services_sources,
  presentation_sources,
  resources,
]

all_deps = [
  gtk4_dep,
  libadwaita_dep,
  libcurl_dep,
  sqlite3_dep,
  libnotify_dep,
  threads_dep,
  json_glib_dep,
  openssl_dep,
  math_dep,
  libtorrent_dep,
]

executable('vxap-downloader',
  all_sources,
  dependencies: all_deps,
  include_directories: include_directories('src'),
  install: true,
)

# Install data files
install_data(
  'data/org.vaxp.downloader.desktop',
  install_dir: get_option('datadir') / 'applications',
)

install_data(
  'data/icons/org.vaxp.downloader.svg',
  install_dir: get_option('datadir') / 'icons' / 'hicolor' / 'scalable' / 'apps',
)
