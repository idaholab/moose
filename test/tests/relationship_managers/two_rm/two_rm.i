[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 8
  ny = 8

  output_ghosting = true
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[AuxVariables]
  [proc]
    order = 'CONSTANT'
    family = 'MONOMIAL'
  []
[]

[AuxKernels]
  [proc]
    type = ProcessorIDAux
    variable = proc
    execute_on = 'initial'
  []
[]

[UserObjects]
  [evaluable_uo0]
    type = TwoRMTester
    execute_on = 'initial'
    element_side_neighbor_layers = 2
    rank = 0
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]
