from options import *

test = { INPUT : 'nemesis_test.i',
         EXODIFF : ['out.e.4.0', 'out.e.4.1', 'out.e.4.2', 'out.e.4.3'],
         MIN_PARALLEL : 4,
         MAX_PARALLEL : 4,
         MESH_MODE : ['PARALLEL'],
         SKIP : 'Libmesh Bug #1365'
         }
