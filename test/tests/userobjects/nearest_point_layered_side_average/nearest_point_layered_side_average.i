[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 40
  ny = 10
  nz = 10
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
    type = Diffusion
    variable = u
  [../]
[]

[AuxKernels]
  [./np_layered_average]
    type = SpatialUserObjectAux
    variable = np_layered_average
    execute_on = timestep_end
    user_object = npla
    boundary = 'bottom top'
  [../]
[]

[UserObjects]
  [./npla]
    type = NearestPointLayeredSideAverage
    direction = x
    points='0.25 0 0.25 0.75 0 0.25 0.25 0 0.75 0.75 0 0.75'
    num_layers = 10
    variable = u
    execute_on = linear
    boundary = 'bottom top'
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./one]
    type = DirichletBC
    variable = u
    boundary = 'right back top'
    value = 1
  [../]
[]



[Executioner]
  type = Steady

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
