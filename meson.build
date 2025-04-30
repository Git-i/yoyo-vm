project('yoyo-vm', 'cpp',
        version : '1.0.0',
        default_options : ['warning_level=3', 'cpp_std=c++20'])

yoyo_vm = library('yoyo_vm', [
        'src/yoyo_vm/vm.cpp',
        'src/yoyo_vm/assembler.cpp',
        'src/yoyo_vm/emitter.cpp',
        'src/yoyo_vm/disassembler.cpp',
    ],
    include_directories:[
        'include/'
    ],
    cpp_args: ['-DYOYO_VM_DLL']
)

yoyo_vm_dep = declare_dependency(include_directories: 'include/', link_with: yoyo_vm)
