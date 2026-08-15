[Mesh]
  [primary]
    type = GeneratedMeshGenerator
    nx = 8
    ny = 8
    dim = 2
    xmin = -1
    ymin = -1
    xmax = 1
    ymax = 1
  []
  [cutter]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
    xmin = -0.3
    ymin = -0.3
    xmax = 0.3
    ymax = 0.3
  []
  [cut]
    type = XYCutMeshByMeshGenerator
    input = primary
    cutter = cutter
    mode = REMOVE_INSIDE
    cutting_type = CUT_ELEM_POLY
    new_boundary_id = 20
  []
  [to_tri]
    type = ElementsToSimplicesConverter
    input = cut
  []
[]
