from options import *

multi_test = { INPUT : 'multismoothcircleIC_test.i',
               EXODIFF : ['multismoothcircleIC_test_out.e'],
               SCALE_REFINE : 1
               }

lattice_test = { INPUT : 'latticesmoothcircleIC_test.i',
                 EXODIFF : ['latticesmoothcircleIC_test_out.e'],
                 SCALE_REFINE : 1
                 }

specified_test = { INPUT : 'specifiedsmoothcircleIC_test.i',
                 EXODIFF : ['specifiedsmoothcircleIC_test_out.e'],
                 SCALE_REFINE : 1
                 }

