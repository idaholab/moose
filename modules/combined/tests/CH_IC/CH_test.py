from options import *

circleIC_test = { INPUT : 'CH_CircleIC_test.i',
                  EXODIFF : ['circle.e-s002'],
                  GROUP : 'adaptive'}

crossIC_test = { INPUT : 'CH_CrossIC_test.i',
                 EXODIFF : ['cross.e-s002'],
                 GROUP : 'adaptive'}

rndcircleIC_test = { INPUT : 'CH_RndCircleIC_test.i',
                     EXODIFF : ['rnd_circle.e-s002'],
                     GROUP : 'adaptive'}

boxIC_test = { INPUT : 'CH_BndingBoxIC_test.i',
               EXODIFF : ['box.e-s002'],
               GROUP : 'adaptive'}

rndboxIC_test = { INPUT : 'CH_RndBndingBoxIC_test.i',
                  EXODIFF : ['rndbox.e-s002'],
                  GROUP : 'adaptive'}

