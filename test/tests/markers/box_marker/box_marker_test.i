[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  nz = 0
  zmax = 0
  elem_type = QUAD4
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Adaptivity]
  [./Markers]
    [./box]
      type = BoxMarker
      bottom_left = '0.3 0.3 0'
      top_right = '0.6 0.6 0'
      inside = refine
      outside = do_nothing
    [../]
  [../]
[]

[Outputs]
  exodus = true
[]
