# This test checks that an error is generated when the user supplies initial
# volume fraction values that are outside the range of the remapper.

[GlobalParams]
  gravity_vector = '0 0 0'

  initial_T_liquid = 300.0
  initial_T_vapor  = 300.0
  initial_p_liquid = 1.0e5
  initial_p_vapor  = 1.0e5
  initial_vel_liquid = 0
  initial_vel_vapor = 0

  # An error should be generated because the next 2 lines are inconsistent:
  # the initial vapor volume fraction function, 0.0001, is not in the range
  # (0.001, 0.999).
  initial_alpha_vapor = 0.0001
  alpha_vapor_bounds = '0.001 0.999'

  closures = simple
[]

[FluidProperties]
  [./eos]
    type = StiffenedGas7EqnFluidProperties
  [../]
[]

[Components]
  [./pipe]
    type = FlowChannel2Phase
    fp = eos
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 5
    A = 1
    f = 0
    f_interface = 0
  [../]

  [./inlet]
    type = SolidWall
    input = 'pipe:in'
  [../]

  [./outlet]
    type = SolidWall
    input = 'pipe:out'
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  start_time = 0
  end_time = 1
  dt = 1
  abort_on_solve_fail = true

  nl_rel_tol = 1e-7
  nl_abs_tol = 1e-4
  nl_max_its = 10

  l_tol = 1e-3
  l_max_its = 30
[]
