[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
[]

[Variables]
  [power_density]
    family = L2_LAGRANGE
    order = FIRST
  []
[]

[Functions]
  [pwr_func]
    type = ParsedFunction
    expression = '1e3*x*(1-x)+5e2'
  []
[]

[Kernels]
  [diff]
    type = Reaction
    variable = power_density
  []

  [coupledforce]
    type = BodyForce
    variable = power_density
    function = pwr_func
  []
[]

[Postprocessors]
  [pwr_avg]
    type = ElementAverageValue
    block = '0'
    variable = power_density
    execute_on = 'initial timestep_end'
  []
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart '
  petsc_options_value = 'hypre boomeramg 100'

  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-12
[]

[MultiApps]
  [sub]
    type = FullSolveMultiApp
    app_type = MooseTestApp
    positions = '0 0 0'
    input_files  = sub_L2_Lagrange.i
    execute_on = 'timestep_end'
  []
[]

[Transfers]
  [p_to_sub]
    type = MultiAppShapeEvaluationTransfer
    source_variable = power_density
    variable = power_density
    to_multi_app = sub
    execute_on = 'timestep_end'
  []
[]

[Outputs]
  exodus = true
  perf_graph = true
[]
