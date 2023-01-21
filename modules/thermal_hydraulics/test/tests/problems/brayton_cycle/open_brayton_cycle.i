# This input file is used to demonstrate a simple open-air Brayton cycle using
# a compressor, turbine, shaft, motor, and generator.
# The flow length is divided into 5 segments as illustrated below, where
#   - "(I)" denotes the inlet
#   - "(C)" denotes the compressor
#   - "(T)" denotes the turbine
#   - "(O)" denotes the outlet
#   - "*" denotes a fictitious junction
#
#                  Heated section
# (I)-----(C)-----*--------------*-----(T)-----(O)
#      1       2         3          4       5
#
# Initially the fluid is at rest at ambient conditions, the shaft speed is zero,
# and no heat transfer occurs with the system.
# The transient is controlled as follows:
#   * 0   - 100 s: motor ramps up torque linearly from zero
#   * 100 - 200 s: motor ramps down torque linearly to zero, HTC ramps up linearly from zero.
#   * 200 - 300 s: (no changes; should approach steady condition)

I_motor = 1.0
motor_torque_max = 400.0

I_generator = 1.0
generator_torque_per_shaft_speed = -0.00025

motor_ramp_up_duration = 100.0
motor_ramp_down_duration = 100.0
post_motor_time = 100.0
t1 = ${motor_ramp_up_duration}
t2 = ${fparse t1 + motor_ramp_down_duration}
t3 = ${fparse t2 + post_motor_time}

D1 = 0.15
D2 = ${D1}
D3 = ${D1}
D4 = ${D1}
D5 = ${D1}

A1 = ${fparse 0.25 * pi * D1^2}
A2 = ${fparse 0.25 * pi * D2^2}
A3 = ${fparse 0.25 * pi * D3^2}
A4 = ${fparse 0.25 * pi * D4^2}
A5 = ${fparse 0.25 * pi * D5^2}

L1 = 10.0
L2 = ${L1}
L3 = ${L1}
L4 = ${L1}
L5 = ${L1}

x1 = 0.0
x2 = ${fparse x1 + L1}
x3 = ${fparse x2 + L2}
x4 = ${fparse x3 + L3}
x5 = ${fparse x4 + L4}

x2_minus = ${fparse x2 - 0.001}
x2_plus = ${fparse x2 + 0.001}
x5_minus = ${fparse x5 - 0.001}
x5_plus = ${fparse x5 + 0.001}

n_elems1 = 10
n_elems2 = ${n_elems1}
n_elems3 = ${n_elems1}
n_elems4 = ${n_elems1}
n_elems5 = ${n_elems1}

A_ref_comp = ${fparse 0.5 * (A1 + A2)}
V_comp = ${fparse A_ref_comp * 1.0}
I_comp = 1.0

A_ref_turb = ${fparse 0.5 * (A4 + A5)}
V_turb = ${fparse A_ref_turb * 1.0}
I_turb = 1.0

c0_rated_comp = 351.6925137
rho0_rated_comp = 1.146881112

rated_mfr = 0.25

speed_rated_rpm = 96000
speed_rated = ${fparse speed_rated_rpm * 2 * pi / 60.0}

speed_initial = 0

eff_comp = 0.79
eff_turb = 0.843

T_hot = 1000
T_ambient = 300
p_ambient = 1e5

[GlobalParams]
  orientation = '1 0 0'
  gravity_vector = '0 0 0'

  initial_p = ${p_ambient}
  initial_T = ${T_ambient}
  initial_vel = 0
  initial_vel_x = 0
  initial_vel_y = 0
  initial_vel_z = 0

  fp = fp_air
  closures = closures
  f = 0

  scaling_factor_1phase = '1 1 1e-5'
  scaling_factor_rhoV = 1
  scaling_factor_rhouV = 1
  scaling_factor_rhovV = 1
  scaling_factor_rhowV = 1
  scaling_factor_rhoEV = 1e-5

  rdg_slope_reconstruction = none
[]

[Functions]
  [motor_torque_fn]
    type = PiecewiseLinear
    x = '0 ${t1} ${t2}'
    y = '0 ${motor_torque_max} 0'
  []
  [motor_power_fn]
    type = ParsedFunction
    expression = 'torque * speed'
    symbol_names = 'torque speed'
    symbol_values = 'motor_torque shaft:omega'
  []
  [generator_torque_fn]
    type = ParsedFunction
    expression = 'slope * t'
    symbol_names = 'slope'
    symbol_values = '${generator_torque_per_shaft_speed}'
  []
  [generator_power_fn]
    type = ParsedFunction
    expression = 'torque * speed'
    symbol_names = 'torque speed'
    symbol_values = 'generator_torque shaft:omega'
  []
  [htc_wall_fn]
    type = PiecewiseLinear
    x = '0 ${t1} ${t2}'
    y = '0 0 1e3'
  []
[]

[FluidProperties]
  [fp_air]
    type = IdealGasFluidProperties
    emit_on_nan = none
  []
[]

[Closures]
  [closures]
    type = Closures1PhaseSimple
  []
[]

[Components]
  [shaft]
    type = Shaft
    connected_components = 'motor compressor turbine generator'
    initial_speed = ${speed_initial}
  []
  [motor]
    type = ShaftConnectedMotor
    inertia = ${I_motor}
    torque = 0 # controlled
  []
  [generator]
    type = ShaftConnectedMotor
    inertia = ${I_generator}
    torque = generator_torque_fn
  []

  [inlet]
    type = InletStagnationPressureTemperature1Phase
    input = 'pipe1:in'
    p0 = ${p_ambient}
    T0 = ${T_ambient}
  []
  [pipe1]
    type = FlowChannel1Phase
    position = '${x1} 0 0'
    length = ${L1}
    n_elems = ${n_elems1}
    A = ${A1}
  []
  [compressor]
    type = ShaftConnectedCompressor1Phase
    position = '${x2} 0 0'
    inlet = 'pipe1:out'
    outlet = 'pipe2:in'
    A_ref = ${A_ref_comp}
    volume = ${V_comp}

    omega_rated = ${speed_rated}
    mdot_rated = ${rated_mfr}
    c0_rated = ${c0_rated_comp}
    rho0_rated = ${rho0_rated_comp}

    speeds = '0.5208 0.6250 0.7292 0.8333 0.9375'
    Rp_functions = 'rp_comp1 rp_comp2 rp_comp3 rp_comp4 rp_comp5'
    eff_functions = 'eff_comp1 eff_comp2 eff_comp3 eff_comp4 eff_comp5'

    min_pressure_ratio = 1.0

    speed_cr_I = 0
    inertia_const = ${I_comp}
    inertia_coeff = '${I_comp} 0 0 0'

    # assume no shaft friction
    speed_cr_fr = 0
    tau_fr_const = 0
    tau_fr_coeff = '0 0 0 0'
  []
  [pipe2]
    type = FlowChannel1Phase
    position = '${x2} 0 0'
    length = ${L2}
    n_elems = ${n_elems2}
    A = ${A2}
  []
  [junction2_3]
    type = JunctionOneToOne1Phase
    connections = 'pipe2:out pipe3:in'
  []
  [pipe3]
    type = FlowChannel1Phase
    position = '${x3} 0 0'
    length = ${L3}
    n_elems = ${n_elems3}
    A = ${A3}
  []
  [junction3_4]
    type = JunctionOneToOne1Phase
    connections = 'pipe3:out pipe4:in'
  []
  [pipe4]
    type = FlowChannel1Phase
    position = '${x4} 0 0'
    length = ${L4}
    n_elems = ${n_elems4}
    A = ${A4}
  []
  [turbine]
    type = ShaftConnectedCompressor1Phase
    position = '${x5} 0 0'
    inlet = 'pipe4:out'
    outlet = 'pipe5:in'
    A_ref = ${A_ref_turb}
    volume = ${V_turb}

    treat_as_turbine = true

    omega_rated = ${speed_rated}
    mdot_rated = ${rated_mfr}
    c0_rated = ${c0_rated_comp}
    rho0_rated = ${rho0_rated_comp}

    speeds = '0 0.5208 0.6250 0.7292 0.8333 0.9375'
    Rp_functions = 'rp_turb0 rp_turb1 rp_turb2 rp_turb3 rp_turb4 rp_turb5'
    eff_functions = 'eff_turb1 eff_turb1 eff_turb2 eff_turb3 eff_turb4 eff_turb5'

    min_pressure_ratio = 1.0

    speed_cr_I = 0
    inertia_const = ${I_turb}
    inertia_coeff = '${I_turb} 0 0 0'

    # assume no shaft friction
    speed_cr_fr = 0
    tau_fr_const = 0
    tau_fr_coeff = '0 0 0 0'
  []
  [pipe5]
    type = FlowChannel1Phase
    position = '${x5} 0 0'
    length = ${L5}
    n_elems = ${n_elems5}
    A = ${A5}
  []
  [outlet]
    type = Outlet1Phase
    input = 'pipe5:out'
    p = ${p_ambient}
  []

  [heating]
    type = HeatTransferFromSpecifiedTemperature1Phase
    flow_channel = pipe3
    T_wall = ${T_hot}
    Hw = htc_wall_fn
  []
[]

[ControlLogic]
  [motor_ctrl]
    type = TimeFunctionComponentControl
    component = motor
    parameter = torque
    function = motor_torque_fn
  []
[]

[Postprocessors]
  [heating_rate]
    type = ADHeatRateConvection1Phase
    block = 'pipe3'
    T = T
    T_wall = T_wall
    Hw = Hw
    P_hf = P_hf
    execute_on = 'INITIAL TIMESTEP_END'
  []

  [motor_torque]
    type = RealComponentParameterValuePostprocessor
    component = motor
    parameter = torque
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [motor_power]
    type = FunctionValuePostprocessor
    function = motor_power_fn
    execute_on = 'INITIAL TIMESTEP_END'
    indirect_dependencies = 'motor_torque shaft:omega'
  []

  [generator_torque]
    type = ShaftConnectedComponentPostprocessor
    quantity = torque
    shaft_connected_component_uo = generator:shaftconnected_uo
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [generator_power]
    type = FunctionValuePostprocessor
    function = generator_power_fn
    execute_on = 'INITIAL TIMESTEP_END'
    indirect_dependencies = 'generator_torque shaft:omega'
  []

  [shaft_speed]
    type = ScalarVariable
    variable = 'shaft:omega'
    execute_on = 'INITIAL TIMESTEP_END'
  []

  [p_in_comp]
    type = PointValue
    variable = p
    point = '${x2_minus} 0 0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [p_out_comp]
    type = PointValue
    variable = p
    point = '${x2_plus} 0 0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [p_ratio_comp]
    type = ParsedPostprocessor
    pp_names = 'p_in_comp p_out_comp'
    function = 'p_out_comp / p_in_comp'
    execute_on = 'INITIAL TIMESTEP_END'
  []

  [p_in_turb]
    type = PointValue
    variable = p
    point = '${x5_minus} 0 0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [p_out_turb]
    type = PointValue
    variable = p
    point = '${x5_plus} 0 0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [p_ratio_turb]
    type = ParsedPostprocessor
    pp_names = 'p_in_turb p_out_turb'
    function = 'p_in_turb / p_out_turb'
    execute_on = 'INITIAL TIMESTEP_END'
  []

  [mfr_comp]
    type = ADFlowJunctionFlux1Phase
    boundary = pipe1:out
    connection_index = 0
    equation = mass
    junction = compressor
  []
  [mfr_turb]
    type = ADFlowJunctionFlux1Phase
    boundary = pipe4:out
    connection_index = 0
    equation = mass
    junction = turbine
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
  scheme = 'bdf2'

  end_time = ${t3}
  dt = 0.1
  abort_on_solve_fail = true

  solve_type = NEWTON
  nl_rel_tol = 1e-50
  nl_abs_tol = 1e-11
  nl_max_its = 15

  l_tol = 1e-4
  l_max_its = 10
[]

[Outputs]
  [csv]
    type = CSV
    file_base = 'open_brayton_cycle'
    execute_vector_postprocessors_on = 'INITIAL'
  []
  [console]
    type = Console
    show = 'shaft_speed p_ratio_comp p_ratio_turb compressor:pressure_ratio turbine:pressure_ratio'
  []
[]

[Functions]
  # compressor pressure ratio
  [rp_comp1]
    type = PiecewiseLinear
    data_file = 'rp_comp1.csv'
    x_index_in_file = 0
    y_index_in_file = 1
    format = columns
    extrap = true
  []
  [rp_comp2]
    type = PiecewiseLinear
    data_file = 'rp_comp2.csv'
    x_index_in_file = 0
    y_index_in_file = 1
    format = columns
    extrap = true
  []
  [rp_comp3]
    type = PiecewiseLinear
    data_file = 'rp_comp3.csv'
    x_index_in_file = 0
    y_index_in_file = 1
    format = columns
    extrap = true
  []
  [rp_comp4]
    type = PiecewiseLinear
    data_file = 'rp_comp4.csv'
    x_index_in_file = 0
    y_index_in_file = 1
    format = columns
    extrap = true
  []
  [rp_comp5]
    type = PiecewiseLinear
    data_file = 'rp_comp5.csv'
    x_index_in_file = 0
    y_index_in_file = 1
    format = columns
    extrap = true
  []

  # compressor efficiency
  [eff_comp1]
    type = ConstantFunction
    value = ${eff_comp}
  []
  [eff_comp2]
    type = ConstantFunction
    value = ${eff_comp}
  []
  [eff_comp3]
    type = ConstantFunction
    value = ${eff_comp}
  []
  [eff_comp4]
    type = ConstantFunction
    value = ${eff_comp}
  []
  [eff_comp5]
    type = ConstantFunction
    value = ${eff_comp}
  []

  # turbine pressure ratio
  [rp_turb0]
    type = ConstantFunction
    value = 1
  []
  [rp_turb1]
    type = PiecewiseLinear
    data_file = 'rp_turb1.csv'
    x_index_in_file = 0
    y_index_in_file = 1
    format = columns
    extrap = true
  []
  [rp_turb2]
    type = PiecewiseLinear
    data_file = 'rp_turb2.csv'
    x_index_in_file = 0
    y_index_in_file = 1
    format = columns
    extrap = true
  []
  [rp_turb3]
    type = PiecewiseLinear
    data_file = 'rp_turb3.csv'
    x_index_in_file = 0
    y_index_in_file = 1
    format = columns
    extrap = true
  []
  [rp_turb4]
    type = PiecewiseLinear
    data_file = 'rp_turb4.csv'
    x_index_in_file = 0
    y_index_in_file = 1
    format = columns
    extrap = true
  []
  [rp_turb5]
    type = PiecewiseLinear
    data_file = 'rp_turb5.csv'
    x_index_in_file = 0
    y_index_in_file = 1
    format = columns
    extrap = true
  []

  # turbine efficiency
  [eff_turb1]
    type = ConstantFunction
    value = ${eff_turb}
  []
  [eff_turb2]
    type = ConstantFunction
    value = ${eff_turb}
  []
  [eff_turb3]
    type = ConstantFunction
    value = ${eff_turb}
  []
  [eff_turb4]
    type = ConstantFunction
    value = ${eff_turb}
  []
  [eff_turb5]
    type = ConstantFunction
    value = ${eff_turb}
  []
[]
