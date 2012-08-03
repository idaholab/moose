from options import *

test_hermite_converge_neumann = { INPUT : 'hermite_converge_neumann.i',
                  EXODIFF : ['hermite_converge_neumann_out.e-s003'],
                  SCALE_REFINE : 5,
                  SKIP : "Q-rule 15 not avail"
                  }

test_hermite_converge_periodic = { INPUT : 'hermite_converge_periodic.i',
                  EXODIFF : ['hermite_converge_periodic_out.e-s003'],
                  SCALE_REFINE : 5,
                  SKIP : "Q-rule 15 not avail"
                  }
