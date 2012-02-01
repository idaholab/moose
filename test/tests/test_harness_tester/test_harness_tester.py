from options import *

# In addition to testing the checks in the harness
# we will stress the dependencies a bit here too

test_compiler_check= { INPUT : '2d_diffusion_test.i',
                       EXODIFF : ['out.e'],
                       COMPILER : ['GCC'],
 }

test_platform_check= { INPUT : '2d_diffusion_test.i',
                       EXODIFF : ['out.e'],
                       PLATFORM : ['DARWIN'],
                       PREREQ : 'test_compiler_check'
 }

test_petsc_check= { INPUT : '2d_diffusion_test.i',
                    EXODIFF : ['out.e'],
                    PETSC_VERSION : ['3.1'],
                    PREREQ : 'test_compiler_check'
 }

test_parmesh_check= { INPUT : '2d_diffusion_test.i',
                      EXODIFF : ['out.e'],
                      MESH_MODE : ['SERIAL'],
                      PREREQ : 'test_platform_check'
 }

test_combined= { INPUT : '2d_diffusion_test.i',
                 EXODIFF : ['out.e'],
                 COMPILER : ['GCC'],
                 MESH_MODE : ['SERIAL'],
                 PLATFORM : ['DARWIN'],
                 PREREQ : 'test_parmesh_check'
 }


