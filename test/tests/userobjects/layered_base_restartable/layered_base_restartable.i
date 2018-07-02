[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 5
  ny = 5
  nz = 5
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./np_layered_average]
    order = CONSTANT
    family = MONOMIAL
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
[]

[AuxKernels]
  [./np_layered_average]
    type = SpatialUserObjectAux
    variable = np_layered_average
    execute_on = 'timestep_begin'
    user_object = npla
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 10
  [../]
  [./one]
    type = DirichletBC
    variable = u
    boundary = 'right back top'
    value = 12
  [../]
[]

[UserObjects]
  [./npla]
    type = NearestPointLayeredAverage
    direction = y
    points = '0.25 0 0.25 0.75 0 0.25 0.25 0 0.75 0.75 0 0.75'
    num_layers = 10
    variable = u
    execute_on = 'timestep_end'
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 8
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
