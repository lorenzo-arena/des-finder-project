project('des-finder-seq', 'c',
  version : '0.1',
  default_options : ['warning_level=3'])

incdir = include_directories('../common')

sources = ['main.c',
           '../common/log.c',
           '../common/stopwatch.c',
           '../common/common.c']

# add_global_arguments('-DTRACE=1', language : 'c')

executable('des-finder-seq',
           [sources],
           install : true,
           include_directories : incdir,
           link_args : '-lcrypt')
