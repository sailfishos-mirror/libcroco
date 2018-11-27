libtest_utils = static_library('test-utils', 'cr-test-utils.c',
                               dependencies: libcroco_dep)

foreach i : [0, 1, 2, 3, 4, 5, 6]
  name = 'test@0@-main'.format(i)
  test(name, executable(name, name + '.c',
                        link_with: libtest_utils,
                        dependencies: libcroco_dep))
endforeach
