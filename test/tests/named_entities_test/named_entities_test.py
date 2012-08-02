from options import *

test_names = { INPUT : 'named_entities_test.i',
               EXODIFF : ['named_entities_test_out.e']
             }

test_periodic_names = { INPUT : 'periodic_bc_names_test.i',
                        EXODIFF : ['periodic_bc_names_test_out.e'],
                        MAX_THREADS : 1,
                        MAX_PARALLEL : 1
                      }

on_the_fly_test = { INPUT : 'name_on_the_fly.i',
                    EXODIFF : ['name_on_the_fly_out.e'],
                   }
