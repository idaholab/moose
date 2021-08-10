[Mesh]
  [./square]
    type = GeneratedMeshGenerator
    nx = 16
    ny = 16
    nz = 16
    dim = 3
  [../]

  parallel_type = distributed
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [ghosting0]
    order = CONSTANT
    family = MONOMIAL
  []
  [ghosting1]
    order = CONSTANT
    family = MONOMIAL
  []
  [ghosting2]
    order = CONSTANT
    family = MONOMIAL
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
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    preset = false
    boundary = 3
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    preset = false
    boundary = 1
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'NEWTON'
[]

[Outputs]
  nemesis = true
[]
