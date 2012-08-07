from options import *

test_hier_1_1d = { INPUT : 'hier-1-1d.i',
                   EXODIFF : ['hier-1-1d_out.e'],
                   MAX_PARALLEL : 1,
                  }

test_hier_1_2d = { INPUT : 'hier-1-2d.i',
                   EXODIFF : ['hier-1-2d_out.e'],
                   MAX_PARALLEL : 1
                  }

test_hier_1_3d = { INPUT : 'hier-1-3d.i',
                   EXODIFF : ['hier-1-3d_out.e'],
                   ABS_ZERO : 1e-9
                  }

test_hier_2_1d = { INPUT : 'hier-2-1d.i',
                   EXODIFF : ['hier-2-1d_out.e']
                  }

test_hier_2_2d = { INPUT : 'hier-2-2d.i',
                   EXODIFF : ['hier-2-2d_out.e'],
                   ABS_ZERO : 1e-9
                  }

test_hier_2_3d = { INPUT : 'hier-2-3d.i',
                   EXODIFF : ['hier-2-3d_out.e']
                  }

test_hier_3_1d = { INPUT : 'hier-3-1d.i',
                   EXODIFF : ['hier-3-1d_out.e'],
                   MAX_PARALLEL : 4
                  }

test_hier_3_2d = { INPUT : 'hier-3-2d.i',
                   EXODIFF : ['hier-3-2d_out.e'],
                   ABS_ZERO : 1e-8
                  }

test_hier_3_3d = { INPUT : 'hier-3-3d.i',
                   EXODIFF : ['hier-3-3d_out.e'],
                   ABS_ZERO : 1e-9
                  }
