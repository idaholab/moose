[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
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
  type = Transient
  num_steps = 1
  dt = 1

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Output]
  linear_residuals = true
  output_initial = true
  exodus = true
  perf_log = true
[]

[MultiApps]
  [./pp_sub]
    app_type = MooseTestApp
    positions = '0.5 0.5 0 0.7 0.7 0'
    execute_on = timestep
    type = TransientMultiApp
    input_files = pp_sub.i
  [../]
[]

[Transfers]
  [./sample_pp_transfer]
    source_variable = u
    direction = to_multiapp
    postprocessor = from_master
    execute_on = timestep
    type = MultiAppVariableValueSamplePostprocessorTransfer
    multi_app = pp_sub
  [../]
[]

