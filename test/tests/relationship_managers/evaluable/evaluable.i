[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 8
  ny = 8
[]

[GlobalParams]
  order = CONSTANT
  family = MONOMIAL
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[AuxVariables]
  [evaluable0]
  []
  [evaluable1]
  []
  [evaluable2]
  []
  [proc]
  []
[]

[AuxKernels]
  [evaluable0]
    type = RMAux
    variable = evaluable0
    rm_user_object = evaluable_uo0
    execute_on = initial
  []
  [evaluable1]
    type = RMAux
    variable = evaluable1
    rm_user_object = evaluable_uo1
    execute_on = initial
  []
  [evaluable2]
    type = RMAux
    variable = evaluable2
    rm_user_object = evaluable_uo2
    execute_on = initial
  []
  [proc]
    type = ProcessorIDAux
    variable = proc
    execute_on = initial
  []
[]

[UserObjects]
  [evaluable_uo0]
    type = AlgebraicRMTester
    execute_on = initial
    element_side_neighbor_layers = 2
    show_evaluable = true
    rank = 0
  []
  [evaluable_uo1]
    type = AlgebraicRMTester
    execute_on = initial
    element_side_neighbor_layers = 2
    show_evaluable = true
    rank = 1
  []
  [evaluable_uo2]
    type = AlgebraicRMTester
    execute_on = initial
    element_side_neighbor_layers = 2
    show_evaluable = true
    rank = 2
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
