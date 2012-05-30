from options import *

test = { INPUT : 'elastic_patch.i',
         EXODIFF : ['elastic_patch_out.e']
         }

test2Procs = { INPUT : 'elastic_patch.i',
               EXODIFF : ['elastic_patch_out.e'],
               MIN_PARALLEL : '2',
               PREREQ : 'test',
               PLATFORM : ['DARWIN'] }

test_quadratic = { INPUT : 'elastic_patch_quadratic.i',
                   EXODIFF : ['elastic_patch_quadratic_out.e']
                   }

test_rz = { INPUT : 'elastic_patch_rz.i',
            EXODIFF : ['elastic_patch_rz_out.e'],
            SCALE_REFINE : 1
            }

test_rz_large_strain = { INPUT : 'elastic_patch_rz_large_strain.i',
                         EXODIFF : ['elastic_patch_rz_large_strain_out.e'],
                         SCALE_REFINE : 3
                       }

test_plane_strain = { INPUT : 'elastic_patch_plane_strain.i',
                      EXODIFF : ['elastic_patch_plane_strain_out.e'],
                      SCALE_REFINE : 1
                    }

test_plane_strain_large_strain = { INPUT : 'elastic_patch_plane_strain_large_strain.i',
                                   EXODIFF : ['elastic_patch_plane_strain_large_strain_out.e'],
                                   SCALE_REFINE : 3
                                 }
