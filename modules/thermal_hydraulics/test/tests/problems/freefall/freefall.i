# Tests acceleration of a fluid due to gravity. The flow exiting the bottom
# of the flow channel enters the top, so the flow should uniformly accelerate
# at the rate of acceleration due to gravity.

acceleration = -10.0
dt = 0.1
num_steps = 5
time = ${fparse num_steps * dt}

# The expected velocity is the following:
#   u = a * t
#     = -10 * 0.5
#     = -5

[GlobalParams]
  gravity_vector = '0 0 ${acceleration}'

  initial_p = 1e5
  initial_T = 300
  initial_vel = 0

  scaling_factor_1phase = '1 1 1e-5'

  closures = simple_closures
[]

[FluidProperties]
  [fp]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    cv = 1816
    q = -1.167e6
    q_prime = 0
    p_inf = 1e9
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
    position = '0 0 0'
    orientation = '0 0 1'
    length = 1
    n_elems = 100
    A = 1
    f = 0
    fp = fp
  []

  [junction]
    type = JunctionOneToOne1Phase
    connections = 'pipe:in pipe:out'
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

  scheme = bdf2
  end_time = ${time}
  dt = ${dt}
  num_steps = ${num_steps}
  abort_on_solve_fail = true

  solve_type = NEWTON
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-8
  nl_max_its = 10
  l_tol = 1e-3
  l_max_its = 10
[]

[Postprocessors]
  [vel_avg]
    type = ElementAverageValue
    variable = 'vel'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Outputs]
  velocity_as_vector = false
  [out]
    type = CSV
    execute_on = 'FINAL'
  []
[]
