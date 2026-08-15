[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 4
    ny = 4
    nz = 4
    xmin = -1
    ymin = -1
    zmin = -1
    elem_type = HEX8
  []
  [lsc]
    type = CutMeshByLevelSetGenerator
    input = gmg
    cut_interface = polyhedra
    level_set = 'x+y+z-0.3'
    cut_face_name = cut
  []
  [to_tet]
    type = ElementsToTetrahedronsConverter
    input = lsc
  []
[]
