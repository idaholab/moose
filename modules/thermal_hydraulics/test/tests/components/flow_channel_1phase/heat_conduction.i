# This input file is used to test that heat conduction occurs in a flow channel.
# The problem setup consists of a closed pipe full of air with an initial temperature
# discontinuity. Heat should travel from the hot side of the step to the cold step.

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[Functions]
  [T_ic_fn]
    type = PiecewiseConstant
    axis = x
    x = '0 0.5'
    y = '300 400'
  []
[]

[Components]
  [pipe]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    gravity_vector = '0 0 0'
    length = 1.0
    n_elems = 10
    A = 0.1

    initial_T = T_ic_fn
    initial_p = 1e5
    initial_vel = 0

    enable_heat_conduction = true

    fp = fp
    closures = simple_closures
    f = 0

    scaling_factor_1phase = '1 1 1e-5'
  []
  [left_wall]
    type = SolidWall1Phase
    input = 'pipe:in'
  []
  [right_wall]
    type = SolidWall1Phase
    input = 'pipe:out'
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

  start_time = 0
  dt = 10.0
  num_steps = 5

  solve_type = NEWTON
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-8
  nl_max_its = 30

  l_tol = 1e-3
  l_max_its = 10

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  exodus = true
  velocity_as_vector = false
  show = 'p T vel'
[]
