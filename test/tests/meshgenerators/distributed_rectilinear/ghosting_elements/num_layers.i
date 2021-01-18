
[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [gmg]
    type = DistributedRectilinearMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    partition="linear"
    num_side_layers = 2
  []
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
  [evaluable0]
    order = CONSTANT
    family = MONOMIAL
  []
  [evaluable1]
    order = CONSTANT
    family = MONOMIAL
  []
  [evaluable2]
    order = CONSTANT
    family = MONOMIAL
  []
  [proc]
    order = CONSTANT
    family = MONOMIAL
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

[Variables]
  [./u]
  [../]

  [./disp_x]
  [../]

  [./disp_y]
  [../]
[]

[Kernels]
  [./diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
  [../]
  [./time]
    type = TimeDerivative
    variable = u
  [../]
  [./diff_x]
    type = Diffusion
    variable = disp_x
  [../]

  [./diff_y]
    type = Diffusion
    variable = disp_y
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]

  [./left_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = -0.01
  [../]

  [./right_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.01
  [../]

  [./left_y]
    type = DirichletBC
    variable = disp_y
    boundary = left
    value = -0.01
  [../]

  [./right_y]
    type = DirichletBC
    variable = disp_y
    boundary = right
    value = 0.01
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 3
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
