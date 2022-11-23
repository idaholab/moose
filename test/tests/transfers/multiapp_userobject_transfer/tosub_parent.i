[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 20
  ny = 20
  nz = 20
  # The MultiAppUserObjectTransfer object only works with ReplicatedMesh
  parallel_type = replicated
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./layered_average_value]
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
  [./layered_aux]
    type = SpatialUserObjectAux
    variable = layered_average_value
    execute_on = timestep_end
    user_object = layered_average
  [../]
[]

[BCs]
  [./bottom]
    type = DirichletBC
    variable = u
    boundary = bottom
    value = 0
  [../]
  [./top]
    type = DirichletBC
    variable = u
    boundary = top
    value = 1
  [../]
[]

[UserObjects]
  [./layered_average]
    type = LayeredAverage
    variable = u
    direction = y
    num_layers = 4
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 1

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [./sub_app]
    execute_on = timestep_end
    positions = '0.3 0.1 0.3 0.7 0.1 0.3'
    type = TransientMultiApp
    input_files = tosub_sub.i
    app_type = MooseTestApp
  [../]
[]

[Transfers]
  [./layered_transfer]
    user_object = layered_average
    variable = multi_layered_average
    type = MultiAppUserObjectTransfer
    to_multi_app = sub_app
    skip_coordinate_collapsing = true
  [../]
  [./element_layered_transfer]
    user_object = layered_average
    variable = element_multi_layered_average
    type = MultiAppUserObjectTransfer
    to_multi_app = sub_app
    skip_coordinate_collapsing = true
  [../]
[]
