[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
  parallel_type = replicated
[]

[Variables]
  [power_density]
    family = L2_LAGRANGE
    order = FIRST
  []
[]

[AuxVariables]
  [./multi_layered_average]
    family = LAGRANGE
    order = FIRST
  [../]
[]

[UserObjects]
  [./multi_layered_average]
    type = LayeredAverage
    variable = power_density
    direction = y
    num_layers = 4
  [../]
[]

[AuxKernels]
  [./layered_aux]
    type = SpatialUserObjectAux
    variable = multi_layered_average
    execute_on = 'nonlinear TIMESTEP_END'
    user_object = multi_layered_average
  [../]
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
  [layered_avg]
    type = ElementAverageValue
    block = '0'
    variable = multi_layered_average
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
    type = MultiAppUserObjectTransfer
    user_object = multi_layered_average
    variable = power_density
    to_multi_app = sub
    execute_on = 'timestep_end'
  []
[]

[Outputs]
  exodus = true
  perf_graph = true
[]
