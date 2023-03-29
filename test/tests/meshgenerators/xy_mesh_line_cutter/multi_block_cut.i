[Mesh]
  [ccmg]
    type = ConcentricCircleMeshGenerator
    num_sectors = 6
    radii = '1 2 4 6'
    rings = '1 2 2 3 2'
    has_outer_square = on
    pitch = 15
    preserve_volumes = false
    smoothing_max_it = 3
  []
  [ext]
      type = RenameBoundaryGenerator
      input = ccmg
      old_boundary = '1 2 3 4'
      new_boundary = '100 100 100 100'
  []
  [mlc]
    type = XYMeshLineCutter
    input = ext
    cut_line_params = '1 -2 0'
    new_boundary_id = 20
    input_mesh_external_boundary_id = 100
  []
[]
