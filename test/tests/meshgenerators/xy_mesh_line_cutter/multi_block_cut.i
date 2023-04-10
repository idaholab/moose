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
  [interface12]
    type = SideSetsBetweenSubdomainsGenerator
    input = ext
    primary_block = '1'
    paired_block = '2'
    new_boundary = '12'
  []
  [interface23]
    type = SideSetsBetweenSubdomainsGenerator
    input = interface12
    primary_block = '2'
    paired_block = '3'
    new_boundary = '23'
  []
  [interface34]
    type = SideSetsBetweenSubdomainsGenerator
    input = interface23
    primary_block = '3'
    paired_block = '4'
    new_boundary = '34'
  []
  [interface45]
    type = SideSetsBetweenSubdomainsGenerator
    input = interface34
    primary_block = '4'
    paired_block = '5'
    new_boundary = '45'
  []
  [mlc]
    type = XYMeshLineCutter
    input = interface45
    cut_line_params = '1 -2 2'
    new_boundary_id = 20
    input_mesh_external_boundary_id = 100
  []
[]
