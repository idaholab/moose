[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 10
  ny = 10
[]

[Variables]
  [v]
  []
[]

[AuxVariables]
  [u]
  []
  [w]
  []
[]

[Kernels]
  [time_derivative]
    type = TimeDerivative
    variable = v
  []
  [diffusion]
    type = Diffusion
    variable = v
  []
  [source]
    type = CoupledForce
    variable = v
    v = u
  []
[]

[BCs]
  [dirichlet0]
    type = DirichletBC
    variable = v
    boundary = '0'
    value = 0
  []
  [dirichlet]
    type = DirichletBC
    variable = v
    boundary = '2'
    value = 100
  []
[]

[Postprocessors]
  [avg_u]
    type = ElementAverageValue
    variable = u
    execute_on = 'initial linear'
  []
  [avg_v]
    type = ElementAverageValue
    variable = v
    execute_on = 'initial linear'
  []
  [avg_w]
    type = ElementAverageValue
    variable = w
    execute_on = 'initial linear'
  []
[]

[Executioner]
  type = Transient

  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart '
  petsc_options_value = 'hypre boomeramg 100'

  end_time = 0.1
  dt = 0.02
  # picard_max_its = 5
  # accept_on_max_picard_iteration = true
  # nl_rel_tol = 1e-8
  # nl_abs_tol = 1e-9
  # picard_rel_tol = 1e-3
  # picard_abs_tol = 1e-7
[]


[MultiApps]
  [level2-]
    type = TransientMultiApp
    app_type = MooseTestApp
    positions = '0 0 0'
    input_files = sub_level2.i
    execute_on = 'timestep_end'
    # sub_cycling = true
  []
[]

[Transfers]
  [v_to_sub]
    type = MultiAppMeshFunctionTransfer
    direction = to_multiapp
    source_variable = v
    variable = v
    multi_app = level2-
    execute_on = 'timestep_end'
  []
  [w_from_sub]
    type = MultiAppMeshFunctionTransfer
    direction = from_multiapp
    source_variable = w
    variable = w
    multi_app = level2-
    execute_on = 'timestep_end'
  []
[]

[Outputs]
  exodus = true
  csv = true
  perf_graph = true
  # print_linear_residuals = false
[]
