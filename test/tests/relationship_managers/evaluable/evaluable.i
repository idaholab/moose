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
    type = ElementUOAux
    variable = evaluable0
    element_user_object = evaluable_uo0
    field_name = "evaluable"
    execute_on = initial
  []
  [evaluable1]
    type = ElementUOAux
    variable = evaluable1
    element_user_object = evaluable_uo1
    field_name = "evaluable"
    execute_on = initial
  []
  [evaluable2]
    type = ElementUOAux
    variable = evaluable2
    element_user_object = evaluable_uo2
    field_name = "evaluable"
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
    type = ElemSideNeighborLayersTester
    execute_on = initial
    element_side_neighbor_layers = 2
    rank = 0
  []
  [evaluable_uo1]
    type = ElemSideNeighborLayersTester
    execute_on = initial
    element_side_neighbor_layers = 2
    rank = 1
  []
  [evaluable_uo2]
    type = ElemSideNeighborLayersTester
    execute_on = initial
    element_side_neighbor_layers = 2
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
