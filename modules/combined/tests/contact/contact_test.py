from options import *

pressurePenalty_test = { INPUT : 'pressurePenalty.i',
                         EXODIFF : ['pressurePenalty_out.e'],
                         CUSTOM_CMP : 'pressure.exodiff'
                       }

pressureAugLag_test = { INPUT : 'pressureAugLag.i',
                        EXODIFF : ['pressureAugLag_out.e'],
                        CUSTOM_CMP : 'pressure.exodiff',
                        PETSC_VERSION : ['>=3.1']
                      }
