project('tg2', 'c',
        version : '1.0.0',
        default_options : ['warning_level=3', 'c_std=c99'])

if host_machine.system() == 'windows'
    raylib_dep = declare_dependency(link_args:['-L.././engine/external/windows_mingw_w64', '-lraylib', '-lwinmm'])
else
    raylib_dep = declare_dependency(link_args:['-L.././engine/external/linux_amd64', '-lraylib'])
endif

source_files = [
    'src/app.c',
    'src/ega.c',
    'src/entity.c',
    'src/level.c',
    'src/monster.c',
    'src/player.c',
    'src/neo.c',
    'src/trace.c',
    'src/util.c',
    'src/res/res.c'
]

include_directories = [
    'engine/src',
    'engine/external/raylib',
    'src/'
]

tg2 = executable('tg2', source_files, install : true,
                dependencies: [raylib_dep],
                include_directories : include_directories,
                c_args: ['-DPLATFORM_DESKTOP'],
                link_args: ['-lm']
                )

test('test', tg2)