[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 8
  ny = 8

  # We are testing geometric ghosted functors
  # so we have to use distributed mesh
  parallel_type = distributed
[]

[GlobalParams]
  order = CONSTANT
  family = MONOMIAL
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [ghosting0]
  []
  [ghosting1]
  []
  [ghosting2]
  []
  [proc]
  []
[]

[AuxKernels]
  [ghosting0]
    type = ElementUOAux
    variable = ghosting0
    element_user_object = ghosting_uo0
    field_name = "ghosted"
    execute_on = initial
  []
  [ghosting1]
    type = ElementUOAux
    variable = ghosting1
    element_user_object = ghosting_uo1
    field_name = "ghosted"
    execute_on = initial
  []
  [ghosting2]
    type = ElementUOAux
    variable = ghosting2
    element_user_object = ghosting_uo2
    field_name = "ghosted"
    execute_on = initial
  []
  [proc]
    type = ProcessorIDAux
    variable = proc
    execute_on = initial
  []
[]

[UserObjects]
  [ghosting_uo0]
    type = ElemSideNeighborLayersTester
    execute_on = initial
    element_side_neighbor_layers = 2
    rank = 0
  []
  [ghosting_uo1]
    type = ElemSideNeighborLayersTester
    execute_on = initial
    element_side_neighbor_layers = 2
    rank = 1
  []
  [ghosting_uo2]
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
