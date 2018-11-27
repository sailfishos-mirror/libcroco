project('libcroco', 'c',
  version : '0.6.13',
  meson_version : '>=0.48')

croco_api_major = '0'
croco_api_minor = '6'
croco_api_version = croco_api_major + '.' + croco_api_minor

# For historical reasons, these are not a function of the actual project version
# current, revision, age
croco_ltversions = [3, 1, 0]
croco_ltversion = '@0@.@1@.@2@'.format(croco_ltversions[0], croco_ltversions[2], croco_ltversions[1])
croco_osxversions = '@0@.@1@.0'.format(croco_ltversions[0] + 1, croco_ltversions[1])

croco_version = meson.project_version()
croco_versions = croco_version.split('.')

cc = meson.get_compiler('c')

glib_dep = dependency('glib-2.0', version : '>=2.0',
                      fallback: ['glib', 'libglib_dep'])
libxml2_dep = dependency('libxml-2.0', version : '>=2.4.23',
                         fallback: ['libxml2', 'xml2lib_dep'])

# Compiler flags
if cc.get_id() == 'gcc' or cc.get_id() == 'clang'
  warning_c_args = [
    '-Wall',
    '-Wextra',
    '-Wunused',
    '-Wreturn-type',
    '-Wswitch',
    '-Wcomment',
    '-Wtrigraphs',
    '-Wchar-subscripts',
    '-Wparentheses',
    '-Winline',
    '-Wredundant-decls',
    '-Wformat-nonliteral',
    '-Werror=format-security',
    '-Wsign-compare',
    '-Werror=implicit-function-declaration',
    '-Wpointer-arith',
    '-Wwrite-strings',
    '-Wstrict-prototypes',
    '-Waggregate-return',
    '-Wcast-align',
    '-Wimplicit',
    '-Wuninitialized',
    '-Wmissing-prototypes',
    '-Wmissing-declarations',
    '-Wnested-externs',
    '-Wpacked',
    '-Wmissing-format-attribute',
    '-Wshadow',
    '-Wstrict-aliasing=2',
    '-Winit-self',
    '-Wunsafe-loop-optimizations',
    '-Wdeclaration-after-statement',
    '-Wold-style-definition',
    '-Wno-missing-field-initializers',
    '-Wno-unused-parameter',
    '-fno-common',
    '-Wno-switch-enum',
  ]
  warning_c_link_args = []
  if get_option('bsymbolic_functions')
    warning_c_link_args += ['-Wl,-Bsymbolic-functions']
  endif
else
  warning_c_args = []
  warning_c_link_args = []
endif
add_project_arguments(cc.get_supported_arguments(warning_c_args), language: 'c')
add_project_link_arguments(cc.get_supported_link_arguments(warning_c_link_args), language: 'c')

debug_cflags = []
buildtype = get_option('buildtype')
if buildtype.startswith('debug')
  debug_cflags += ['-DG_ENABLE_DEBUG']
elif buildtype == 'release'
  debug_cflags += ['-DG_DISABLE_CAST_CHECKS']
endif
add_project_arguments(debug_cflags, language: 'c')

# For generating libcroco.pc
pcdata = configuration_data()
pcdata.set('prefix', get_option('prefix'))
pcdata.set('exec_prefix', get_option('prefix'))
pcdata.set('libdir', join_paths('${prefix}', get_option('libdir')))
pcdata.set('includedir', join_paths('${prefix}', get_option('includedir')))
pcdata.set('VERSION', croco_version)
pcdata.set('LIBCROCO_MAJOR_VERSION', croco_api_major)
pcdata.set('LIBCROCO_MINOR_VERSION', croco_api_minor)
configure_file(input : 'libcroco.pc.in',
               output : 'libcroco-@0@.pc'.format(croco_api_version),
               configuration : pcdata,
               install_dir : join_paths(get_option('libdir'),
                                        'pkgconfig'))
subdir('src')
subdir('csslint')
subdir('tests')
if find_program('gtkdoc-scan', required : get_option('gtk_doc')).found()
  subdir('docs/reference')
else
  message('Not building documentation as gtk-doc was not found')
endif
