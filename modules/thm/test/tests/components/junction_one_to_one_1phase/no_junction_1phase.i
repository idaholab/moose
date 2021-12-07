# This input file is used to generate gold values for the junction_one_to_one_1phase.i
# test. Unlike junction_one_to_one_1phase.i, this file has no junction in the
# middle of the domain. In junction_one_to_one_1phase.i, the post-processors are
# side post-processors, but in this input file, side post-processors cannot be
# used to obtain the solution at these positions since there are no sides there.
# Therefore, the solution is sampled at points just to the left and right of
# the middle to obtain the piecewise constant solution values to either side of
# the interface.

[GlobalParams]
  gravity_vector = '0 0 0'

  closures = simple_closures
[]

[Functions]
  [p_ic_fn]
    type = PiecewiseConstant
    axis = x
    direction = right
    x = '0.5 1.0'
    y = '1.0 0.1'
  []

  [T_ic_fn]
    type = PiecewiseConstant
    axis = x
    direction = right
    x = '0.5 1.0'
    y = '1.4 1.12'
  []
[]

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
    gamma = 1.4
    molar_mass = 11.64024372
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[Components]
  [left_boundary]
    type = FreeBoundary1Phase
    input = 'channel:in'
  []

  [channel]
    type = FlowChannel1Phase

    fp = fp

    position = '0 0 0'
    orientation = '1 0 0'
    length = 1.0
    n_elems = 100
    A = 1.0

    initial_T = T_ic_fn
    initial_p = p_ic_fn
    initial_vel = 0

    f = 0
  []

  [right_boundary]
    type = FreeBoundary1Phase
    input = 'channel:out'
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

  solve_type = NEWTON
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-8
  nl_max_its = 60

  l_tol = 1e-4

  start_time = 0.0
  dt = 1e-3
  num_steps = 5
  abort_on_solve_fail = true
[]

[Postprocessors]
  [rhoA_left]
    type = PointValue
    variable = rhoA
    point = '0.4999 0 0'
    execute_on = 'initial timestep_end'
  []
  [rhouA_left]
    type = PointValue
    variable = rhouA
    point = '0.4999 0 0'
    execute_on = 'initial timestep_end'
  []
  [rhoEA_left]
    type = PointValue
    variable = rhoEA
    point = '0.4999 0 0'
    execute_on = 'initial timestep_end'
  []
  [rhoA_right]
    type = PointValue
    variable = rhoA
    point = '0.5001 0 0'
    execute_on = 'initial timestep_end'
  []
  [rhouA_right]
    type = PointValue
    variable = rhouA
    point = '0.5001 0 0'
    execute_on = 'initial timestep_end'
  []
  [rhoEA_right]
    type = PointValue
    variable = rhoEA
    point = '0.5001 0 0'
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  csv = true
  file_base = 'junction_one_to_one_1phase_out'
  execute_on = 'initial timestep_end'
[]
