[GlobalParams]
  initial_p = 1e5
  initial_T = 300
  initial_vel = 0

  closures = simple_closures
  fp = fp
  f = 0
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
  [sw1]
    type = SolidWall1Phase
    input = fch1:in
  []

  [fch1]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 2
    A = 1
  []

  [test_comp]
    type = ShaftConnectedTestComponent
    position = '1 0 0'
    volume = 1
    connections = 'fch1:out fch2:in'
  []

  [fch2]
    type = FlowChannel1Phase
    position = '1 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 2
    A = 1
  []

  [sw2]
    type = SolidWall1Phase
    input = fch2:out
  []

  [shaft]
    type = Shaft
    connected_components = 'test_comp'
    initial_speed = 1
  []

[]

[Preconditioning]
  [SMP_PJFNK]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  start_time = 0
  num_steps = 1
  abort_on_solve_fail = true

  solve_type = 'newton'
  line_search = 'basic'
  petsc_options_iname = '-snes_test_err'
  petsc_options_value = '1e-9'

  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-6
  nl_max_its = 15

  l_tol = 1e-4
  l_max_its = 10
[]
