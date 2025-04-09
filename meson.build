project('yoyo-vm', 'cpp',
        version : '1.0.0',
        default_options : ['warning_level=3', 'cpp_std=c++20'])

yoyo_vm = executable('yoyo_vm', [
        'main.cpp',
        'src/yoyo_vm/vm.cpp',
        'src/yoyo_vm/assembler.cpp'
])

test('test', yoyo_vm)