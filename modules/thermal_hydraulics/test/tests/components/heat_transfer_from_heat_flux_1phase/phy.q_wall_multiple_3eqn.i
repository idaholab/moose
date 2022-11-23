# Tests that energy conservation is satisfied in 1-phase flow when there are
# multiple heat transfer components connected to the same pipe, using specified
# wall heat flux.
#
# This problem has 2 wall heat flux sources, each with differing parameters.
# Solid wall boundary conditions are imposed such that there should be no flow,
# and the solution should be spatially uniform. With no other sources, the
# energy balance is
#   (rho*e*A)^{n+1} = (rho*e*A)^n + dt * [(q1*P1) + (q2*P2)]
# Note that spatial integration is dropped here due to spatial uniformity, and
# E has been replaced with e since velocity should be zero.
#
# For the initial conditions
#   p = 100 kPa
#   T = 300 K
# the density and specific internal energy should be
#   rho = 1359.792245 kg/m^3
#   e = 1.1320645935e+05 J/kg
#
# With the following heat source parameters:
#   q1 = 10 MW/m^2     P1 = 0.2 m
#   q2 = 20 MW/m^2     P2 = 0.4 m
# and A = 1 m^2 and dt = 2 s, the new energy solution value should be
#   (rho*e*A)^{n+1} = 1359.792245 * 1.1320645935e+05 * 1 + 2 * (10e6 * 0.2 + 20e6 * 0.4)
#                   = 173937265.50803775 J/m
#

[GlobalParams]
  gravity_vector = '0 0 0'

  initial_T = 300
  initial_p = 100e3
  initial_vel = 0

  closures = simple_closures
[]

[FluidProperties]
  [fp]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    q = -1167e3
    q_prime = 0
    p_inf = 1.e9
    cv = 1816
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
    A = 1
    f = 0

    # length and number of elements should be arbitrary for the test
    length = 10
    n_elems = 1
  []

  [ht1]
    type = HeatTransferFromHeatFlux1Phase
    flow_channel = pipe
    q_wall = 10e6
    P_hf = 0.2
    Hw = 1
  []

  [ht2]
    type = HeatTransferFromHeatFlux1Phase
    flow_channel = pipe
    q_wall = 20e6
    P_hf = 0.4
    Hw = 1
  []

  [left]
    type = SolidWall1Phase
    input = 'pipe:in'
  []

  [right]
    type = SolidWall1Phase
    input = 'pipe:out'
  []
[]

[Preconditioning]
  [preconditioner]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  start_time = 0
  dt = 2
  num_steps = 1
  abort_on_solve_fail = true

  solve_type = 'NEWTON'
  line_search = 'basic'
  nl_rel_tol = 0
  nl_abs_tol = 1e-6
  nl_max_its = 5

  l_tol = 1e-10
  l_max_its = 10
[]

[Postprocessors]
  [rhoEA_predicted]
    type = ElementAverageValue
    variable = rhoEA
    block = pipe
  []
[]

[Outputs]
  [out]
    type = CSV
    show = 'rhoEA_predicted'
    execute_on = 'final'
  []
[]
