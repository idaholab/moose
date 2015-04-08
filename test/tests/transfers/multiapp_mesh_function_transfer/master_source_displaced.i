[Mesh]
  # The MultiAppMeshFunctionTransfer doesn't work with ParallelMesh.
  # 2145 for more information.
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  distribution = serial
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./transferred_u]
  [../]
  [./elemental_transferred_u]
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
[]

[Executioner]
  # Preconditioned JFNK (default)
  type = Transient
  num_steps = 1
  dt = 1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  output_initial = true
  exodus = true
  print_linear_residuals = true
  print_perf_log = true
[]

[MultiApps]
  [./sub]
    positions = '.099 .099 0 .599 .599 0 0.599 0.099 0'
    type = TransientMultiApp
    app_type = MooseTestApp
    input_files = sub.i
  [../]
[]

[Transfers]
  [./from_sub]
    source_variable = sub_u
    direction = from_multiapp
    variable = transferred_u
    type = MultiAppMeshFunctionTransfer
    multi_app = sub
    displaced_source_mesh = true
  [../]
  [./elemental_from_sub]
    source_variable = sub_u
    direction = from_multiapp
    variable = elemental_transferred_u
    type = MultiAppMeshFunctionTransfer
    multi_app = sub
    displaced_source_mesh = true
  [../]
[]
