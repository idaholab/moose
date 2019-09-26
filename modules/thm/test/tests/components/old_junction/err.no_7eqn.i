[GlobalParams]
  initial_p_liquid = 1e6
  initial_p_vapor = 1e6
  initial_T_liquid = 300
  initial_T_vapor = 500
  initial_vel_liquid = 0
  initial_vel_vapor = 0
  initial_alpha_vapor = 0.1

  closures = simple
[]

[FluidProperties]
  [./fp]
    type = StiffenedGas7EqnFluidProperties
  [../]
[]

[Components]
  [./pipe1]
    type = FlowChannel2Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 100
    A = 1
    f = 0.1
    f_interface = 0
    Hw_liquid = 0
    Hw_vapor = 0
    fp = fp
  [../]

  [./pipe2]
    type = FlowChannel2Phase
    position = '1 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 100
    A = 1
    f = 0.1
    f_interface = 0
    Hw_liquid = 0
    Hw_vapor = 0
    fp = fp
  [../]

  [./junction]
    type = OldJunction
    connections = 'pipe1:out pipe2:in'
    A_ref = 1e-4
    K = '0 0'
  [../]

  [./inlet]
    type = InletStagnationPressureTemperature2Phase
    input = 'pipe1:in'
    p0_liquid = 1e6
    T0_liquid = 300
    p0_vapor = 1e6
    T0_vapor = 300
    alpha_vapor = 0.1
  [../]

  [./outlet]
    type = Outlet2Phase
    input = 'pipe2:out'
    p_liquid = 1e6
    p_vapor = 1e6
  [../]
[]

[Executioner]
  type = Transient
[]
