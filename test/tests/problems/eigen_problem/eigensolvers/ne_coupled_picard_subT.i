[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 10
  ymin = 0
  ymax = 10
  elem_type = QUAD4
  nx = 8
  ny = 8
[]

[Variables]
  [./T]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./power]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.1
  [../]
[]

[Kernels]
  [./diff_T]
    type = Diffusion
    variable = T
  [../]
  [./src_T]
    type = CoupledForce
    variable = T
    v = power
  [../]
[]

[BCs]
  [./homogeneousT]
    type = DirichletBC
    variable = T
    boundary = '0 1 2 3'
    value = 0
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-6
  fixed_point_max_its = 20
  fixed_point_rel_tol = 1e-6
[]

[MultiApps]
  [./sub]
    type = FullSolveMultiApp
    keep_solution_during_restore = true
    input_files = ne_coupled_picard_subT_sub.i
    execute_on = timestep_end
  [../]
[]

[Transfers]
  [./T_to_sub]
    type = MultiAppShapeEvaluationTransfer
    to_multi_app = sub
    source_variable = T
    variable = T
    execute_on = timestep_end
  [../]
  [./power_from_sub]
    type = MultiAppShapeEvaluationTransfer
    from_multi_app = sub
    source_variable = power
    variable = power
    execute_on = timestep_end
  [../]
[]

[Outputs]
  csv = true
  exodus =true
  execute_on = 'timestep_end'
[]
