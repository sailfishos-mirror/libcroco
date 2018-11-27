gnome = import('gnome')

gnome.gtkdoc('libcroco',
  main_sgml : 'libcroco-docs.sgml',
  src_dir : libcroco_inc,
  scanobjs_args : ['--type-init-func="g_type_init()"'],
  dependencies : libcroco_dep,
  install : true)
