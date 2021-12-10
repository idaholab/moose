T_in = 523.0
mdot = 10
pout = 7e6

[GlobalParams]
  initial_p = ${pout}
  initial_vel = 1
  initial_T = ${T_in}
  gravity_vector = '0 0 0'
  closures = simple_closures
  n_elems = 5

  scaling_factor_1phase = '1 1e-2 1e-5'
  f = 1
[]

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
    gamma = 1.66
    molar_mass = 0.004
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[Components]
  [inlet_bc]
    type = InletMassFlowRateTemperature1Phase
    input = 'inlet:in'
    m_dot = ${mdot}
    T = ${T_in}
  []

  [inlet]
    type = FlowChannel1Phase
    fp = fp
    position = '0 0 11'
    orientation = '0 0 -1'
    length = 1
    A = 1
  []

  [inlet_plenum]
    type = VolumeJunction1Phase
    position = '0 0 10'
    initial_vel_x = 0
    initial_vel_y = 0
    initial_vel_z = 1
    connections = 'inlet:out bypass:in core_top:in'
    volume = 1
  []

  [bypass]
    type = FlowChannel1Phase
    fp = fp
    position = '2 0 10'
    orientation = '0 0 -1'
    length = 10
    A = 0.01
  []

  [core_top]
    type = FlowChannel1Phase
    fp = fp
    position = '0 0 10'
    orientation = '0 0 -1'
    length = 0.1
    A = 9
  []

  [core_top_bc]
    type = Outlet1Phase
    p = ${pout}
    input = 'core_top:out'
  []

  [core_bottom_bc]
    type = InletMassFlowRateTemperature1Phase
    input = 'core_bottom:in'
    m_dot = ${mdot}
    T = ${T_in}
  []

  [core_bottom]
    type = FlowChannel1Phase
    fp = fp
    position = '0 0 0.1'
    orientation = '0 0 -1'
    length = 0.1
    A = 9
  []

  [outlet_plenum]
    type = VolumeJunction1Phase
    position = '0 0 0'
    initial_vel_x = 1
    initial_vel_y = 0
    initial_vel_z = 1
    connections = 'bypass:out core_bottom:out outlet:in'
    volume = 1
  []

  [outlet]
    type = FlowChannel1Phase
    fp = fp
    position = '0 0 0'
    orientation = '0 0 -1'
    length = 1
    A = 1
  []

  [outlet_bc]
    type = Outlet1Phase
    p = ${pout}
    input = 'outlet:out'
  []
[]

[ControlLogic]
  [set_core_inlet_pressure]
    type = SetComponentRealValueControl
    component = core_top_bc
    parameter = p
    value = core_inlet_pressure
  []

  [set_core_outlet_mdot]
    type = SetComponentRealValueControl
    component = core_bottom_bc
    parameter = m_dot
    value = core_outlet_mdot
  []

  [set_core_outlet_temperature]
    type = SetComponentRealValueControl
    component = core_bottom_bc
    parameter = T
    value = core_outlet_temperature
  []
[]

[Postprocessors]
  [core_inlet_pressure]
    type = Receiver
    default = ${pout}
  []

  [core_outlet_mdot]
    type = Receiver
    default = ${mdot}
  []

  [core_outlet_temperature]
    type = Receiver
    default = ${T_in}
  []

  [core_outlet_pressure]
    type = SideAverageValue
    variable = p
    boundary = 'core_bottom:in'
    execute_on = 'INITIAL LINEAR TIMESTEP_END'
  []

  [core_inlet_mdot]
    type = SideAverageValue
    variable = rhouA
    boundary = 'core_top:out'
    execute_on = 'INITIAL LINEAR TIMESTEP_END'
  []

  [core_inlet_temperature]
    type = SideAverageValue
    variable = T
    boundary = 'core_top:out'
    execute_on = 'INITIAL LINEAR TIMESTEP_END'
  []

  [bypass_inlet_pressure]
    type = SideAverageValue
    variable = p
    boundary = 'bypass:in'
  []

  [bypass_outlet_pressure]
    type = SideAverageValue
    variable = p
    boundary = 'bypass:out'
  []

  [bypass_pressure_drop]
    type = DifferencePostprocessor
    value1 = bypass_inlet_pressure
    value2 = bypass_outlet_pressure
  []

  [bypass_mdot]
    type = SideAverageValue
    variable = rhouA
    boundary = 'bypass:out'
    execute_on = 'INITIAL LINEAR TIMESTEP_END'
  []

  [inlet_mdot]
    type = SideAverageValue
    variable = rhouA
    boundary = 'inlet:in'
    execute_on = 'INITIAL LINEAR TIMESTEP_END'
  []

  [outlet_mdot]
    type = SideAverageValue
    variable = rhouA
    boundary = 'outlet:out'
    execute_on = 'INITIAL LINEAR TIMESTEP_END'
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
  timestep_tolerance = 1e-6
  start_time = 0
  end_time = 100
  dt = 0.01
  line_search = l2
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-4
  nl_max_its = 25
  l_tol = 1e-3
  l_max_its = 20
  petsc_options = '-snes_converged_reason'
  petsc_options_iname = '-pc_type'
  petsc_options_value = ' lu     '
[]

[Outputs]
  exodus = true
[]
