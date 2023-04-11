[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    nx = 4
    ny = 4
    dim = 2
    xmin = -1
    ymin = -1
  []
  [ext]
    type = SideSetsAroundSubdomainGenerator
    new_boundary = 100
    input = gmg
    block = 0
  []
  [mlc]
    type = XYMeshLineCutter
    input = ext
    cut_line_params = '1 -2 0'
    new_boundary_id = 20
  []
[]
