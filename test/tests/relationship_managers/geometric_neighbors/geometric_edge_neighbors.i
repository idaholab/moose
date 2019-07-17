# This test will show 2 layers of geometric ghosting and 0 layers of evaluable
# ghosting. The 2 layers of geometric ghosting corresponds to the 2 layers we
# have explicitly requested. There is no evaulable ghosting because we have not
# requested any algebraic or coupling functors.

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
  [evaluable0]
    type = ElementUOAux
    variable = evaluable0
    element_user_object = ghosting_uo0
    field_name = "evaluable"
    execute_on = initial
  []
  [evaluable1]
    type = ElementUOAux
    variable = evaluable1
    element_user_object = ghosting_uo1
    field_name = "evaluable"
    execute_on = initial
  []
  [evaluable2]
    type = ElementUOAux
    variable = evaluable2
    element_user_object = ghosting_uo2
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
  [ghosting_uo0]
    type = ElemSideNeighborLayersGeomTester
    execute_on = initial
    element_side_neighbor_layers = 2
    rank = 0
  []
  [ghosting_uo1]
    type = ElemSideNeighborLayersGeomTester
    execute_on = initial
    element_side_neighbor_layers = 2
    rank = 1
  []
  [ghosting_uo2]
    type = ElemSideNeighborLayersGeomTester
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
