from options import *

testnodalpps = { INPUT : 'nodal_max_pps_test.i',
                 EXODIFF : ['nodal_max_pps_test_out.e'],
                 MESH_MODE : ['SERIAL'],
                 GROUP : 'periodic'
}

testnodalnodest = { INPUT : 'nodal_nodeset_pps_test.i',
                    EXODIFF : ['nodal_nodeset_pps_test_out.e']
}
