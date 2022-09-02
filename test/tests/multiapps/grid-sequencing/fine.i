[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 40
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [time]
    type = TimeDerivative
    variable = u
  []
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [rxn]
    type = Reaction
    variable = u
  []
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
  num_steps = 2
  dt = 1

  solve_type = 'PJFNK'
  petsc_options = '-snes_monitor_solution'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [./coarse]
    type = TransientMultiApp
    app_type = MooseTestApp
    execute_on = timestep_begin
    positions = '0 0 0'
    input_files = coarse.i
  [../]
[]

[Transfers]
  [./mesh_function_begin]
    type = MultiAppShapeEvaluationTransfer
    from_multi_app = coarse
    source_variable = u
    variable = u
    execute_on = timestep_begin
  [../]
[]
