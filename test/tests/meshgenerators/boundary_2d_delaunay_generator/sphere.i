[Mesh]
  [sphere]
    type = SphereMeshGenerator
    nr = 2
    radius = 1
  []
  [bdry]
    type = ParsedGenerateSideset
    combinatorial_geometry = 'z>0.5'
    new_sideset_name = 'bdry'
    input = 'sphere'
    included_boundaries = '0'
  []
  [hole]
    type = ParsedGenerateSideset
    combinatorial_geometry = 'z>0.8'
    new_sideset_name = 'hole'
    input = 'bdry'
    included_boundaries = '0'
  []
  [b2dd]
    type = Boundary2DDelaunayGenerator
    input = hole
    boundary_names = 'bdry'
    hole_boundary_names = 'hole'
    use_auto_area_func = true
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Postprocessors]
  [area]
    type = VolumePostprocessor
  []
[]

[Outputs]
  [csv]
    type = CSV
    execute_on = 'FINAL'
  []
[]
