project('libgen', 'c', 
    default_options: ['c_std=c11', 'warning_level=3'],
    license: 'GNU-2.0-only',
    meson_version: '>=0.64',
    version: '0.1.0'
)

cc = meson.get_compiler('c')
argp = cc.find_library('argp', required: false)
curses = dependency('ncursesw', version: '>=6.3')
sqlite = dependency('sqlite3', version: '>=3.34')
deps = [curses, sqlite]
if argp.found()
    deps += [argp]
endif
executable('libgen', 
    sources: ['src/main.c', 'src/libgen.c'], 
    dependencies: deps,
    install: true
)

install_man('doc/libgen.1')
meson.add_install_script('data/install_db.sh')