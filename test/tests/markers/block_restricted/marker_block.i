[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
    xmax = 5
    ymax = 5
  []
  [./lower_block]
    input = gen
    type = SubdomainBoundingBoxGenerator
    top_right = '5 3 0'
    bottom_left = '0 0 0'
    block_id = 0
  [../]
  [./upper_block]
    input = lower_block
    type = SubdomainBoundingBoxGenerator
    top_right = '5 5 0'
    bottom_left = '0 3 0'
    block_id = 1
  [../]
[]

[Adaptivity]
  initial_steps = 2
  initial_marker = marker
  [./Markers]
    [./marker]
      type = UniformMarker
      block = 0
      mark = REFINE
    [../]
  [../]
[]

[Variables]
  [./u]
    initial_condition = 0
  [../]
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
