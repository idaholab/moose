from options import *

circleIC_test = { INPUT : 'CH_CircleIC_test.i',
                  EXODIFF : ['circle_oversample.e'],
                  GROUP : 'adaptive',
                  SCALE_REFINE : 1
                  }

crossIC_test = { INPUT : 'CH_CrossIC_test.i',
                 EXODIFF : ['cross.e-s002'],
                 GROUP : 'adaptive',
                 SCALE_REFINE : 1
                 }

rndcircleIC_test = { INPUT : 'CH_RndCircleIC_test.i',
                     EXODIFF : ['rnd_circle.e-s002'],
                     GROUP : 'adaptive',
                     SCALE_REFINE : 1
                     }

boxIC_test = { INPUT : 'CH_BndingBoxIC_test.i',
               EXODIFF : ['box.e-s002'],
               GROUP : 'adaptive',
               SCALE_REFINE : 1
               }

rndboxIC_test = { INPUT : 'CH_RndBndingBoxIC_test.i',
                  EXODIFF : ['rndbox.e-s004'],
                  GROUP : 'adaptive',
                  SCALE_REFINE : 1
                  }

circleIC_3D_test = { INPUT : 'SmoothCircleIC_3D_test.i',
                  EXODIFF : ['circle_3D.e'],
                  SCALE_REFINE : 1
                  }

