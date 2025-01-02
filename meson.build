project('sober', ['c', 'cpp'], default_options: ['cpp_std=gnu++20', 'warning_level=2'])

cmake = import('cmake')

sober_cpp_arguments = [
  '-O3', '-g3',
]
add_project_arguments(sober_cpp_arguments, language: 'cpp')
add_project_arguments(sober_cpp_arguments, language: 'c')

sober_include_directories = [
  'src',
  'src/vendor/libc-shim/include',
  'src/vendor/rpmalloc/rpmalloc'
]

sober_source_files = [
  'src/vendor/rpmalloc/rpmalloc/rpmalloc.c',

  'src/vendor/libc-shim/src/common.cpp',
  'src/vendor/libc-shim/src/pthreads.cpp',
  'src/vendor/libc-shim/src/semaphore.cpp',
  'src/vendor/libc-shim/src/network.cpp',
  'src/vendor/libc-shim/src/dirent.cpp',
  'src/vendor/libc-shim/src/cstdio.cpp',
  'src/vendor/libc-shim/src/errno.cpp',
  'src/vendor/libc-shim/src/ctype_data.cpp',
  'src/vendor/libc-shim/src/bionic/strlcpy.cpp',
  'src/vendor/libc-shim/src/stat.cpp',
  'src/vendor/libc-shim/src/file_misc.cpp',
  'src/vendor/libc-shim/src/sysconf.cpp',
  'src/vendor/libc-shim/src/system_properties.cpp',
  'src/vendor/libc-shim/src/iorewrite.cpp',
  'src/vendor/libc-shim/src/statvfs.cpp',
  'src/vendor/libc-shim/src/sched.cpp',

  'src/vendor/glad/glad.c',

  'src/libs/syms.cpp',
  'src/libs/libm_syms.cpp',
  'src/libs/android_syms.cpp',
  'src/libs/glesv2_syms.cpp',
  'src/libs/egl_syms.cpp',
  'src/libs/vulkan_syms.cpp',
  'src/libs/log_syms.cpp',
  'src/libs/mediandk_syms.cpp',
  'src/libs/opensles_syms.cpp',
  'src/libs/zlib_syms.cpp',
  'src/libs/pthread_impl.cpp',
  'src/libs/libc_impl.cpp',

  'src/class/android_classes.cpp',
  'src/class/session_reporter_interface.cpp',
  'src/class/native_gl_interface.cpp',
  'src/class/user_java_interface.cpp',
  'src/class/platform_params.cpp',
  'src/class/device_params.cpp',
  'src/class/init_params.cpp',
  'src/class/start_app_params.cpp',
  'src/class/start_game_params.cpp',
  'src/class/java_primitive_wrappers.cpp',
  'src/class/battery_status.cpp',
  'src/class/native_locale_java_interface.cpp',
  'src/class/audio_interface.cpp',
  'src/class/classes.cpp',

  'src/window.cpp',
  'src/keyboard.cpp',
  'src/trampoline.cpp',
  'src/execution_context.cpp',
  'src/jni.cpp',
  'src/app_methods.cpp',
  'src/app_state.cpp',
  'src/main.cpp'
]

liblinker_proj = subproject('liblinker')
liblinker_dep = liblinker_proj.get_variable('liblinker_dep')

curl_dep = dependency('libcurl')
egl_dep = dependency('egl')
zlib_dep = dependency('zlib')
glesv2_dep = dependency('glesv2')
vulkan_dep = dependency('vulkan')
sdl2_dep = dependency('sdl2')
dyncall_dep = dependency('dyncall_s')

dynarmic_opt = cmake.subproject_options()
dynarmic_opt.add_cmake_defines({'DYNARMIC_USE_BUNDLED_EXTERNALS': 'ON'})
dynarmic_proj = cmake.subproject('dynarmic', options: dynarmic_opt)
dynarmic_dep = dynarmic_proj.dependency('dynarmic')
fmt_dep = dynarmic_proj.dependency('fmt')
mcl_dep = dynarmic_proj.dependency('mcl')
zydis_dep = dynarmic_proj.dependency('Zydis')
oaknut_dep = dynarmic_proj.dependency('oaknut')

sober_dependencies = [
  curl_dep,
  egl_dep,
  zlib_dep,
  glesv2_dep,
  vulkan_dep,
  dynarmic_dep,
  dyncall_dep,
  liblinker_dep,
  sdl2_dep,
  dynarmic_dep,
  fmt_dep,
  mcl_dep,
  zydis_dep,
  oaknut_dep,
]

executable('sober', [sober_source_files], dependencies: sober_dependencies, include_directories: sober_include_directories, install: true)
