# Gold value should be the following:
#  htc * (T_wall - T) * P_hf * L

T_wall = 350
T = 300
htc = 50
P_hf = 0.3
L = 2.0

[GlobalParams]
  gravity_vector = '0 0 0'

  closures = simple_closures
[]

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

[Components]
  [left_wall]
    type = SolidWall1Phase
    input = 'pipe:in'
  []

  [pipe]
    type = FlowChannel1Phase
    fp = fp

    position = '0 0 0'
    orientation = '1 0 0'
    length = ${L}
    n_elems = 10
    A = 1

    f = 0.

    initial_p = 1e6
    initial_T = ${T}
    initial_vel = 0
  []

  [right_wall]
    type = SolidWall1Phase
    input = 'pipe:out'
  []

  [heat_flux]
    type = HeatTransferFromSpecifiedTemperature1Phase
    flow_channel = pipe
    Hw = ${htc}
    T_wall = ${T_wall}
    P_hf = ${P_hf}
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

  start_time = 0.0
  dt = 0.01
  num_steps = 0
  abort_on_solve_fail = true

  solve_type = 'PJFNK'
  line_search = 'basic'
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-6
  nl_max_its = 10

  l_tol = 1e-3
  l_max_its = 10
[]

[Postprocessors]
  [heat_rate]
    type = ADHeatRateConvection1Phase
    P_hf = P_hf
    execute_on = 'INITIAL'
  []
[]

[Outputs]
  csv = true
[]
