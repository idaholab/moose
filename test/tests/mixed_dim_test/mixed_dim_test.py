from options import *

onedtwod_test = { INPUT : '1d_2d.i',
                  EXODIFF : ['out.e']
                  }
onedthreed_test = { INPUT : '1d_3d.i',
                    EXODIFF : ['1d_3d_out.e']
                    }

# Meshes with mixed dimension elements can not be refined
