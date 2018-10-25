[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 8
  ny = 8
[]

[GlobalParams]
  order = 'CONSTANT'
  family = 'MONOMIAL'
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
  [geometric0]
    family = MONOMIAL
    order = CONSTANT
  []
  [geometric1]
    family = MONOMIAL
    order = CONSTANT
  []
  [geometric2]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [evaluable0]
    type = GhostingAux
    functor_type = algebraic
    pid = 0
    variable = evaluable0
    execute_on = 'LINEAR TIMESTEP_END INITIAL'
  []
  [evaluable1]
    type = GhostingAux
    functor_type = algebraic
    pid = 1
    variable = evaluable1
    execute_on = 'LINEAR TIMESTEP_END INITIAL'
  []
  [evaluable2]
    type = GhostingAux
    functor_type = algebraic
    pid = 2
    variable = evaluable2
    execute_on = 'LINEAR TIMESTEP_END INITIAL'
  []
  [proc]
    type = ProcessorIDAux
    variable = proc
    execute_on = 'initial'
  []
  [geometric0]
    type = GhostingAux
    pid = 0
    variable = geometric0
    execute_on = 'LINEAR TIMESTEP_END INITIAL'
  []
  [geometric1]
    type = GhostingAux
    pid = 1
    variable = geometric1
    execute_on = 'LINEAR TIMESTEP_END INITIAL'
  []
  [geometric2]
    type = GhostingAux
    pid = 2
    variable = geometric2
    execute_on = 'LINEAR TIMESTEP_END INITIAL'
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
