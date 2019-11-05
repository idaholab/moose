# Tests that error is thrown if one tries to use Junction with the 7-equation model

[GlobalParams]
  gravity_vector = '0 0 0'

  initial_alpha_vapor = 0.3
  initial_p_liquid = 1e5
  initial_T_liquid = 250
  initial_p_vapor = 1e5
  initial_T_vapor = 350
  initial_vel_liquid = 0
  initial_vel_vapor = 0

  closures = simple
[]

[FluidProperties]
  [./eos]
    type = StiffenedGasTwoPhaseFluidProperties
  [../]
[]

[Components]
  [./inlet]
    type = SolidWall
    input = 'pipe1:in'
  [../]

  [./pipe1]
    type = FlowChannel2Phase
    fp = eos

    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    A = 1

    f = 0

    n_elems = 20
  [../]

  [./junction]
    type = Junction
    connections = 'pipe1:out pipe2:in'
    initial_T = 300
    initial_p = 1e5
  [../]

  [./pipe2]
    type = FlowChannel2Phase
    fp = eos

    position = '1 0 0'
    orientation = '1 0 0'
    length = 1
    A = 1

    f = 0

    n_elems = 20
  [../]

  [./outlet]
    type = SolidWall
    input = 'pipe2:out'
  [../]
[]

[Preconditioning]
  [./pc]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  solve_type = 'NEWTON'
  line_search = 'basic'
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-6
  nl_max_its = 10

  l_tol = 1e-10
  l_max_its = 10

  start_time = 0
  end_time = 0.1

  dt = 0.05
  dtmin = 0.05

  [./Quadrature]
    type = GAUSS
    order = SECOND
  [../]
[]
