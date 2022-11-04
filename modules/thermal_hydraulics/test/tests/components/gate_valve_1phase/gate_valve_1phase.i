# This input file is used to test the gate valve component.

# This problem consists of a T junction of 3 pipes. The inlet pipe is one of the
# 2 pipes of the "top" of the T. The other 2 pipes each have a gate valve.
# Initially, one of the 2 outlet pipes has an open valve and the other has a
# closed valve. Later in the transient, the valves gradually open/close to switch
# the outlet flow direction.

p = 1.0e5
T = 300.0
rho = 1.161430436 # @ 1e5 Pa, 300 K

D = 0.1
A = ${fparse pi * D^2 / 4.0}
V_junction = ${fparse pi * D^3 / 4.0}

vel_in = 2.0
m_dot = ${fparse rho * vel_in * A}

t_begin = 0.3
delta_t_open = 0.1

[GlobalParams]
  gravity_vector = '0 0 0'

  closures = simple_closures
  fp = fp

  f = 0.0

  initial_T = ${T}
  initial_p = ${p}
  initial_vel = 0
[]

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
    gamma = 1.4
    molar_mass = 0.02897
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[Functions]
  [pipe3_open_fn]
    type = TimeRampFunction
    initial_value = 1
    final_value = 0
    initial_time = ${t_begin}
    ramp_duration = ${delta_t_open}
  []
  [pipe2_open_fn]
    type = ParsedFunction
    expression = '1 - pipe3_phi'
    symbol_names = 'pipe3_phi'
    symbol_values = 'pipe3_open_fn'
  []
[]

[Components]
  [inlet]
    type = InletMassFlowRateTemperature1Phase
    input = 'pipe1:in'
    m_dot = ${m_dot}
    T = ${T}
  []

  [pipe1]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1.0
    n_elems = 50
    A = ${A}
  []

  [volume_junction]
    type = VolumeJunction1Phase
    position = '1 0 0'
    connections = 'pipe1:out pipe2A:in pipe3A:in'
    volume = ${V_junction}
    initial_vel_x = 0
    initial_vel_y = 0
    initial_vel_z = 0
  []

  [pipe2A]
    type = FlowChannel1Phase
    position = '1 0 0'
    orientation = '0 1 0'
    length = 0.5
    n_elems = 25
    A = ${A}
  []

  [pipe2_valve]
    type = GateValve1Phase
    connections = 'pipe2A:out pipe2B:in'
    open_area_fraction = 0 # (controlled via 'pipe2_valve_control')
  []

  [pipe2B]
    type = FlowChannel1Phase
    position = '1 0.5 0'
    orientation = '0 1 0'
    length = 0.5
    n_elems = 25
    A = ${A}
  []

  [pipe2_outlet]
    type = Outlet1Phase
    input = 'pipe2B:out'
    p = ${p}
  []

  [pipe3A]
    type = FlowChannel1Phase
    position = '1 0 0'
    orientation = '1 0 0'
    length = 0.5
    n_elems = 25
    A = ${A}
  []

  [pipe3_valve]
    type = GateValve1Phase
    connections = 'pipe3A:out pipe3B:in'
    open_area_fraction = 0 # (controlled via 'pipe3_valve_control')
  []

  [pipe3B]
    type = FlowChannel1Phase
    position = '1.5 0 0'
    orientation = '1 0 0'
    length = 0.5
    n_elems = 25
    A = ${A}
  []

  [pipe3_outlet]
    type = Outlet1Phase
    input = 'pipe3B:out'
    p = ${p}
  []
[]

[ControlLogic]
  [pipe2_valve_control]
    type = TimeFunctionComponentControl
    component = pipe2_valve
    parameter = open_area_fraction
    function = pipe2_open_fn
  []
  [pipe3_valve_control]
    type = TimeFunctionComponentControl
    component = pipe3_valve
    parameter = open_area_fraction
    function = pipe3_open_fn
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

  solve_type = PJFNK
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
  nl_max_its = 20

  l_tol = 1e-4

  start_time = 0.0
  end_time = 1.0
  dt = 0.01
  abort_on_solve_fail = true
[]

[Outputs]
  exodus = true
  show = 'p T vel'
  velocity_as_vector = false
  print_linear_residuals = false
  [console]
    type = Console
    max_rows = 1
  []
[]
