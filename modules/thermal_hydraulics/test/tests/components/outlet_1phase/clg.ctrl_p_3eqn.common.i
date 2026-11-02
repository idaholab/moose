[GlobalParams]
  gravity_vector = '0 0 0'

  initial_p = 1e5
  initial_T = 300
  initial_vel = 0.0

  scaling_factor_1phase = '1 1 1e-5'

  closures = simple_closures
[]

[FluidProperties]
  [fp]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    cv = 1816.0
    q = -1.167e6
    p_inf = 1.0e9
    q_prime = 0
    k = 0.5
    mu = 281.8e-6
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[Functions]
  [outlet_p_fn]
    type = PiecewiseLinear
    x = '0   1'
    y = '1e5 1.001e5'
  []
[]

[ChainControls]
  [set_outlet_value_fn]
    type = GetFunctionValueChainControl
    function = outlet_p_fn
  []
  [set_outlet_value]
    type = SetRealValueChainControl
    parameter = component::outlet/p
    value = set_outlet_value_fn:value
  []
[]

[Postprocessors]
  [outlet_p]
    type = RealComponentParameterValuePostprocessor
    component = outlet
    parameter = p
  []
[]

[Preconditioning]
  [pc]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  start_time = 0.0
  dt = 0.25
  num_steps = 5
  abort_on_solve_fail = true

  solve_type = 'NEWTON'
  line_search = 'basic'

  nl_rel_tol = 1e-5
  nl_abs_tol = 1e-6
  nl_max_its = 30

  l_tol = 1e-3
  l_max_its = 100

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  [Quadrature]
    type = GAUSS
    order = SECOND
  []
[]

[Outputs]
  csv = true
[]
