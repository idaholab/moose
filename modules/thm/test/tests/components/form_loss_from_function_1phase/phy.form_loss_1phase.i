# Tests the form loss kernel for 1-phase flow.
#
# This test uses the following parameters and boundary data:
# Inlet: (rho = 996.5563397 kg/m^3, vel = 0.5 m/s)
# Outlet: p_out = 100 kPa
# Length: L = 2 m
# Form loss coefficient: K = 0.5, => K_prime = 0.25 m^-1 (uniform along length)
#
# The inlet pressure is
#
#   p_in = p_out + dp ,
#
# where dp is given by the definition of the form loss coefficient:
#
#   dp = K * 0.5 * rho * u^2
#      = 0.5 * 0.5 * 996.5563397 * 0.5^2
#      = 62.28477123125 Pa
#
# This value is output to CSV.

p_out = 100e3

[GlobalParams]
  initial_p = ${p_out}
  initial_vel = 0.5
  initial_T = 300.0

  gravity_vector = '0 0 0'

  scaling_factor_1phase = '1 1 1e-4'

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

[Components]
  [pipe]
    type = FlowChannel1Phase
    fp = fp
    position = '0 0 0'
    orientation = '1 0 0'
    length = 2
    A = 1
    n_elems = 5

    f = 0
  []
  [form_loss]
    type = FormLossFromFunction1Phase
    flow_channel = pipe
    K_prime = 0.25
  []
  [inlet]
    type = InletDensityVelocity1Phase
    input = 'pipe:in'
    rho = 996.5563397
    vel = 0.5
  []
  [outlet]
    type = Outlet1Phase
    input = 'pipe:out'
    p = ${p_out}
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  scheme = bdf2
  dt = 0.1
  abort_on_solve_fail = true

  solve_type = 'NEWTON'
  line_search = 'basic'
  nl_rel_tol = 1e-8
  nl_abs_tol = 5e-8
  nl_max_its = 10

  l_tol = 1e-3
  l_max_its = 20

  start_time = 0.0
  num_steps = 100

  [Quadrature]
    type = GAUSS
    order = SECOND
  []
[]

[Postprocessors]
  # this is not the right value, should be the value from the inlet ghost cell
  [p_in]
    type = SideAverageValue
    boundary = inlet
    variable = p
    execute_on = TIMESTEP_END
  []
  [p_out]
    type = FunctionValuePostprocessor
    function = ${p_out}
    execute_on = TIMESTEP_END
  []
  [dp]
    type = DifferencePostprocessor
     value1 = p_in
     value2 = p_out
     execute_on = TIMESTEP_END
  []
[]

[Outputs]
  [out]
    type = CSV
    show = 'dp'
    execute_postprocessors_on = final
  []
[]
