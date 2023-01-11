[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  parallel_type = 'replicated'
[]

[AuxVariables]
  [Tf]
  []
[]

[Variables]
  [power_density]
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
    type = Diffusion
    variable = power_density
  []

  [coupledforce]
    type = BodyForce
    variable = power_density
    function = pwr_func
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = power_density
    boundary = left
    value = 50
  []
  [right]
    type = DirichletBC
    variable = power_density
    boundary = right
    value = 1e3
  []
[]

[Postprocessors]
  [pwr_avg]
    type = ElementAverageValue
    variable = power_density
    execute_on = 'initial timestep_end'
  []
  [temp_avg]
    type = ElementAverageValue
    variable = Tf
    execute_on = 'initial final'
  []
  [temp_max]
    type = ElementExtremeValue
    value_type = max
    variable = Tf
    execute_on = 'initial final'
  []
  [temp_min]
    type = ElementExtremeValue
    value_type = min
    variable = Tf
    execute_on = 'initial final'
  []
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart '
  petsc_options_value = 'hypre boomeramg 100'

  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-12

  fixed_point_rel_tol = 1E-7
  fixed_point_abs_tol = 1.0e-07
  fixed_point_max_its = 12
[]

[MultiApps]
  [sub]
    type = FullSolveMultiApp
    app_type = MooseTestApp
    positions = '0   0 0
                 0.5 0 0'
    input_files  = steady_with_sub_sub.i
    execute_on = 'timestep_end'
  []
[]

[Transfers]
  [p_to_sub]
    type = MultiAppProjectionTransfer
    source_variable = power_density
    variable = power_density
    to_multi_app = sub
    execute_on = 'timestep_end'
  []
  [t_from_sub]
    type = MultiAppGeometricInterpolationTransfer
    source_variable = temp
    variable = Tf
    from_multi_app = sub
    execute_on = 'timestep_end'
  []
[]

[Outputs]
  exodus = true
  perf_graph = true
  checkpoint = true
  execute_on = 'INITIAL TIMESTEP_END FINAL'
[]
