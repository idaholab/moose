from options import *

test_rz_tf = { INPUT : 'heat_convection_rz_tf_test.i',
         EXODIFF : ['out_rz_tf.e'],
         ABS_ZERO : 1e-10 }

test_rz = { INPUT : 'heat_convection_rz_test.i',
               EXODIFF : ['out_rz.e'],
               ABS_ZERO : 1e-10 }

test_3d_tf = { INPUT : 'heat_convection_3d_tf_test.i',
            EXODIFF : ['out_3d_tf.e'],
            ABS_ZERO : 1e-10 }

test_3d = { INPUT : 'heat_convection_3d_test.i',
                  EXODIFF : ['out_3d.e'],
                  ABS_ZERO : 1e-10 }

