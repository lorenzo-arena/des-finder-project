project('des-finder-parallel', 'c',
  version : '0.1',
  default_options : ['warning_level=3'])

incdir = include_directories('../common')

sources = ['main.c',
           '../common/log.c',
           '../common/stopwatch.c',
           '../common/common.c']

executable('des-finder-parallel',
           [sources],
           install : true,
           include_directories : incdir,
           link_args : ['-lcrypt', '-pthread'])
