# This test tests that the turbine can startup from rest and reach full power.
# The mass flow rate for the inlet component is ramped up over 10s. The dyno
# component and pid_ctrl controler are used to maintain the turbine's rated shaft
# speed. The turbine should supply ~1e6 W of power to the shaft by the end of the test.

omega_rated = 450
mdot = 5.0
T_in = 1000.0
p_out = 1e6

[GlobalParams]
  f = 1
  scaling_factor_1phase = '0.04 0.04 0.04e-5'
  closures = simple_closures
  n_elems = 20
  initial_T = ${T_in}
  initial_p = ${p_out}
  initial_vel = 0
  initial_vel_x = 0
  initial_vel_y = 0
  initial_vel_z = 0
[]

[FluidProperties]
  [eos]
    type = IdealGasFluidProperties
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[Components]
  [ch_in]
    type = FlowChannel1Phase
    position = '-1 0 0'
    orientation = '1 0 0'
    length = 1
    A = 0.1
    D_h = 1
    fp = eos
  []
  [inlet]
    type = InletMassFlowRateTemperature1Phase
    input = 'ch_in:in'
    m_dot = 0
    T = ${T_in}
  []
  [turbine]
    type = ShaftConnectedTurbine1Phase
    inlet = 'ch_in:out'
    outlet = 'ch_out:in'
    position = '0 0 0'
    scaling_factor_rhoEV = 1e-5
    A_ref = 0.1
    volume = 0.0002
    inertia_coeff = '1 1 1 1'
    inertia_const = 1.61397
    speed_cr_I = 1e12
    speed_cr_fr = 0
    tau_fr_coeff = '0 0 0 0'
    tau_fr_const = 0
    omega_rated = ${omega_rated}
    D_wheel = 0.4
    head_coefficient = head
    power_coefficient = power
    use_scalar_variables = false
  []
  [ch_out]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    A = 0.1
    D_h = 1
    fp = eos
  []
  [outlet]
    type = Outlet1Phase
    input = 'ch_out:out'
    p = ${p_out}
  []

  [dyno]
    type = ShaftConnectedMotor
    inertia = 10
    torque = -450
  []
  [shaft]
    type = Shaft
    connected_components = 'turbine dyno'
    initial_speed = ${omega_rated}
  []
[]


[Functions]
  [head]
    type = PiecewiseLinear
    x = '0 7e-3 1e-2'
    y = '0 15 20'
  []
  [power]
    type = PiecewiseLinear
    x = '0 6e-3 1e-2'
    y = '0 0.05 0.18'
  []

  [mfr_fn]
    type = PiecewiseLinear
    x = '0    10'
    y = '1e-6 ${mdot}'
  []
  [dts]
    type = PiecewiseConstant
    y = '5e-3 1e-2 5e-2 5e-1'
    x = '0 0.5 1 10'
  []
[]

[ControlLogic]
  [mfr_cntrl]
    type = TimeFunctionComponentControl
    component = inlet
    parameter = m_dot
    function = mfr_fn
  []

  [speed_set_point]
    type = GetFunctionValueControl
    function = ${omega_rated}
  []
  [pid_ctrl]
    type = PIDControl
    input = omega
    set_point = speed_set_point:value
    K_i = 2
    K_p = 5
    K_d = 5
    initial_value = -450
  []
  [set_torque_value]
    type = SetComponentRealValueControl
    component = dyno
    parameter = torque
    value = pid_ctrl:output
  []
[]

[Postprocessors]
  [omega]
    type = ScalarVariable
    variable = shaft:omega
    execute_on = 'initial timestep_end'
  []
  [flow_coefficient]
    type = ElementAverageValue
    variable = flow_coeff
    block = 'turbine'
    execute_on = 'initial timestep_end'
  []
  [delta_p]
    type = ElementAverageValue
    variable = delta_p
    block = 'turbine'
    execute_on = 'initial timestep_end'
  []
  [power]
    type = ElementAverageValue
    variable = power
    block = 'turbine'
    execute_on = 'initial timestep_end'
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  scheme = 'implicit-euler'
  start_time = 0
  [TimeStepper]
    type = FunctionDT
    function = dts
  []
  end_time = 20
  abort_on_solve_fail = true

  solve_type = 'PJFNK'
  line_search = 'basic'
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-4
  nl_max_its = 30

  l_tol = 1e-4
  l_max_its = 20
  [Quadrature]
    type = GAUSS
    order = SECOND
  []
[]

[Outputs]
  [console]
    type = Console
    max_rows = 1
  []
  print_linear_residuals = false
[]
