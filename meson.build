project('libgen', 'c', 
    default_options: ['c_std=c11', 'warning_level=2'],
    license: 'GNU-2.0-only',
    meson_version: '>=1.7.0',
    version: '0.2.0'
)

cc = meson.get_compiler('c')
argp = cc.find_library('argp', required: false)
curses = dependency('ncursesw', version: '>=6.5')
sqlite = dependency('sqlite3', version: '>=3.47')
lmagic = dependency('libmagic', version: '>=5.46')
deps = [curses, sqlite, lmagic]
if argp.found()
    deps += [argp]
endif

executable('libgen', 
    sources: ['src/main.c', 'src/libgen.c'], 
    dependencies: deps,
    install: true
)

install_man('doc/libgen.1')
meson.add_install_script('bin/install_db.sh')
