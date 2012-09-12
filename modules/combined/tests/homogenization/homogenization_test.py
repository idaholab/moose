from options import *

shortFiber_test = { INPUT : 'anisoShortFiber.i',
                    EXODIFF : ['anisoShortFiber_out.e']
                  }

longFiber_test = { INPUT : 'anisoLongFiber.i',
                   EXODIFF : ['anisoLongFiber_out.e']
                 }

heatConduction_test = { INPUT : 'heatConduction2D.i',
                        EXODIFF : ['heatConduction2D_out.e'],
                        SKIP : 'Reinvestigate'
                      }
