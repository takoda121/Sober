project('liblinker', 'c', 'cpp', default_options: ['cpp_std=c++20'])

liblinker_project_arguments = [
  '-D__ANDROID_VENDOR__',
  '-include', 'sobercompat.h',
  '-fpermissive',
  '-O2'
]

add_project_arguments(liblinker_project_arguments, language: 'c')
add_project_arguments(liblinker_project_arguments, language: 'cpp')

private_includes = include_directories(
  'internal_includes',
  'external/logging/liblog/include',
  'external/bionic/linker',
  'external/core/libcutils/include',
  'external/libziparchive/include',
  'external/libziparchive/incfs_support/include',
  'src',
)

includes = include_directories(
  'src'
)

linker_source_files = [
  'external/bionic/linker/linker_debuggerd_stub.cpp',
  'external/bionic/libc/bionic/bionic_systrace.cpp',
  'external/bionic/linker/linker_transparent_hugepage_support.cpp',
  'external/bionic/libc/bionic/elf_note.cpp',
  'external/bionic/linker/linker_phdr.cpp',
  'external/bionic/linker/rt.cpp',
  'external/bionic/linker/linker_gdb_support.cpp',
  'external/bionic/libc/bionic/bionic_call_ifunc_resolver.cpp',
  'external/bionic/linker/linker_dlwarning.cpp',
  'external/bionic/linker/dlfcn.cpp',
  'external/bionic/linker/linker_soinfo.cpp',
  'external/bionic/linker/linker.cpp',
  'external/bionic/linker/linker_config.cpp',
  'external/bionic/linker/linker_utils.cpp',
  'external/bionic/linker/linker_debug.cpp',
  'external/bionic/linker/linker_block_allocator.cpp',
  'external/bionic/linker/linker_mapped_file_fragment.cpp',
  'external/bionic/linker/linker_relocate.cpp',
  'external/bionic/linker/linker_namespaces.cpp',
  'external/libbase/mapped_file.cpp',
  'external/bionic/linker/linker_globals.cpp',
  'external/bionic/linker/linker_main.cpp',
  'external/bionic/linker/linker_cfi.cpp',
  'external/bionic/linker/linker_sdk_versions.cpp',
  'external/bionic/linker/linker_logger.cpp',
  'external/libbase/file.cpp',
  'external/libbase/logging.cpp',
  'external/bionic/libc/async_safe/async_safe_log.cpp',
  'external/libbase/stringprintf.cpp',
  'external/libbase/strings.cpp',
  'external/logging/liblog/logger_write.cpp',
  'external/logging/liblog/properties.cpp',
  'external/libbase/threads.cpp',
  'external/libbase/properties.cpp',
  'external/libbase/parsebool.cpp',
  'external/libbase/strings.cpp',
  'external/libziparchive/zip_cd_entry_map.cc',
  'external/libziparchive/zip_archive_stream_entry.cc',
  'external/libziparchive/zip_archive.cc',
  'external/bionic/libdl/libdl.cpp',
  'src/linker.cpp',
]

threads_dep = dependency('threads')
zlib_dep = dependency('zlib')

liblinker_dependencies = [
  threads_dep,
  zlib_dep,
]

liblinker = static_library(
  'linker',
  [linker_source_files],
  include_directories: private_includes,
  dependencies: liblinker_dependencies,
  install: true
)

liblinker_dep = declare_dependency(
  include_directories: includes,
  link_with: liblinker
)
