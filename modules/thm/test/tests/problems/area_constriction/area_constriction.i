# This test features air flowing through a channel whose cross-sectional area
# shrinks to half its value in the right half. Assuming incompressible flow
# conditions, such as having a low Mach number, the velocity should approximately
# double from inlet to outlet.

p_outlet = 1e5

[GlobalParams]
  gravity_vector = '0 0 0'

  initial_T = 300
  initial_p = ${p_outlet}
  initial_vel = initial_vel_fn

  fp = fp
  closures = simple_closures
  f = 0

  scaling_factor_1phase = '1 1 1e-5'
[]

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
    gamma = 1.4
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[Functions]
  [A_fn]
    type = PiecewiseConstant
    axis = x
    direction = right
    x = '0.5 1.0'
    y = '1.0 0.5'
  []
  [initial_vel_fn]
    type = PiecewiseConstant
    axis = x
    direction = right
    x = '0.5 1.0'
    y = '1.0 2'
  []
[]

[Components]
  [inlet]
    type = InletDensityVelocity1Phase
    input = 'pipe:in'
    rho = 1.16263315948279 # rho @ (p = 1e5 Pa, T = 300 K)
    vel = 1
  []

  [pipe]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 100

    A = A_fn
  []

  [outlet]
    type = Outlet1Phase
    input = 'pipe:out'
    p = ${p_outlet}
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

  end_time = 10
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 0.001
    optimal_iterations = 5
    iteration_window = 1
    growth_factor = 1.2
  []

  steady_state_detection = true

  solve_type = PJFNK
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-8
  nl_max_its = 15

  l_tol = 1e-3
  l_max_its = 10
[]

[Outputs]
  exodus = true
  velocity_as_vector = false
  show = 'A rho vel p'
[]
