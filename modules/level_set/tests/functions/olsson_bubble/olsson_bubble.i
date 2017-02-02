[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
[]

[Adaptivity]
  initial_marker = marker
  initial_steps = 2
  max_h_level = 2
  [./Markers]
    [./marker]
      type = BoxMarker
      bottom_left = '0 0 0'
      top_right = '0.5 0.5 0'
      inside = REFINE
      outside = DO_NOTHING
    [../]
  [../]
[]

[AuxVariables]
  [./bubble]
  [../]
[]

[AuxKernels]
  [./bubble_aux]
    type = FunctionAux
    variable = bubble
    function = bubble_func
    execute_on = initial
  [../]
[]


[Functions]
  [./bubble_func]
    type = LevelSetOlssonBubble
    center = '0.25 0.25 0'
    radius = 0.15
  [../]
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  execute_on = 'TIMESTEP_END'
  exodus = true
[]
