bcid_shift = 0

[Mesh]
  [big]
    type = GeneratedMeshGenerator
    dim = 3
    elem_type = TET4
    nx = 1
    ny = 1
    nz = 1
    xmin = -1
    xmax = 1
    ymin = -1
    ymax = 1
    zmin = -1
    zmax = 1
  []
  [h1]
    type = GeneratedMeshGenerator
    dim = 3
    elem_type = TET4
    nx = 1
    ny = 1
    nz = 1
    xmin = -0.7
    xmax = -0.3
    ymin = -0.7
    ymax = -0.3
    zmin = -0.7
    zmax = -0.3
    subdomain_ids = '1'
    boundary_id_offset = 10
    boundary_name_prefix = h1
  []
  [h2]
    type = GeneratedMeshGenerator
    dim = 3
    elem_type = TET4
    nx = 1
    ny = 1
    nz = 1
    xmin = 0.3
    xmax = 0.7
    ymin = 0.3
    ymax = 0.7
    zmin = 0.3
    zmax = 0.7
    subdomain_ids = '2'
    boundary_id_offset = 20
    boundary_name_prefix = h2
  []
  [triang]
    type = XYZDelaunayGenerator
    boundary = 'big'
    holes = 'h1 h2'
    desired_volume = 1
    output_boundary = 'ext'
    hole_boundaries = 'h1 h2'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [ext_area_name]
    type = AreaPostprocessor
    boundary = 'ext'
  []
  [h1_area_name]
    type = AreaPostprocessor
    boundary = 'h1'
  []
  [h2_area_name]
    type = AreaPostprocessor
    boundary = 'h2'
  []
  [ext_area_id]
    type = AreaPostprocessor
    boundary = ${fparse bcid_shift + 0}
  []
  [h1_area_id]
    type = AreaPostprocessor
    boundary = ${fparse bcid_shift + 1}
  []
  [h2_area_id]
    type = AreaPostprocessor
    boundary = ${fparse bcid_shift + 2}
  []
[]

[Outputs]
  [csv]
    type = CSV
    execute_on = 'FINAL'
  []
[]
