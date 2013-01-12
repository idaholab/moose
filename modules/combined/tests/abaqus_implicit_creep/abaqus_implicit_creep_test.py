from options import *

test = { INPUT : 'abaqus_implicit_creep.i',
         EXODIFF : ['out.e'],
         ABS_ZERO : 1e-6,
	 PETSC_VERSION : ['>=3.1.0'],
         LIBRARY_MODE : ['DYNAMIC']
         }

