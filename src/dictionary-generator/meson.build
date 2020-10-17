project('dictionary-generator', 'c',
  version : '0.1',
  default_options : ['warning_level=3'])

incdir = include_directories('../common')

sources = ['main.c', '../common/log.c']

executable('dictionary-generator',
           [sources],
           install : true,
           include_directories : incdir,
           link_args : '-lcrypt')
