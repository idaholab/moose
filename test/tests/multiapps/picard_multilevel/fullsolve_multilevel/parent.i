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
  [u]
  []
[]

[AuxVariables]
  [v]
    initial_condition = 50
  []
[]

[Kernels]
  [diffusion]
    type = Diffusion
    variable = u
  []
  [source]
    type = CoupledForce
    variable = u
    v = v
  []
[]

[BCs]
  [dirichlet0]
    type = DirichletBC
    variable = u
    boundary = '3'
    value = 0
  []
  [dirichlet]
    type = DirichletBC
    variable = u
    boundary = '1'
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
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart '
  petsc_options_value = 'hypre boomeramg 100'

  fixed_point_rel_tol = 1E-3
  fixed_point_abs_tol = 1.0e-05
  fixed_point_max_its = 12
[]

[MultiApps]
  [level1-]
    type = FullSolveMultiApp
    app_type = MooseTestApp
    positions = '0 0 0'
    input_files = sub_level1.i
    execute_on = 'fixedpoint_end'

    # We need to disable restoration within the multiapp so that the previous
    # picard iterate is used as the starting solution within each iteration
    no_restore = false
  []
[]

[Transfers]
  [u_to_sub]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    source_variable = u
    variable = u
    to_multi_app = level1-
    execute_on = 'fixedpoint_end'
  []
  [v_from_sub]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    source_variable = v
    variable = v
    from_multi_app = level1-
    execute_on = 'fixedpoint_end'
  []
[]

[Outputs]
  exodus = true
  perf_graph = true
[]
