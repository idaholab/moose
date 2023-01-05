import numpy as np
import sys
import os
sys.path.append('/Applications/Cubit-15.4/Cubit.app/Contents/MacOS')
sys.path.append(os.getcwd() + "/../meshing")
import cubit
import cubutils as util
import hexagon as hex

cubit.init(['cubit'])


# from Chang 2006 ABTR prelim design
f2f_full = 14.598e-2
rod_pitch = 9.04e-3
f2f_inner = 16 * rod_pitch * np.sin(np.pi/3)

# This mesh creates a single fluid region (Elia approach I)
cubit.cmd('reset')
hex.cubitHexMesh([f2f_full], 0.8, 10, 10, [1])
#util.cubFileHereHelper("test.cub")
util.exodusFileHereHelper("single_region.e")

# This mesh creates an inner and outer region (Elia approach II)
cubit.cmd('reset')
cubit.cmd('reset')
hex.cubitHexMesh([f2f_inner, f2f_full], 0.8, 12, 10, [1, 2])
util.exodusFileHereHelper("two_region.e")

print ("Assembly pitch: ", f2f_full, "Inner region pitch:", f2f_inner)
