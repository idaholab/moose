# This input file models an open, recuperated Brayton cycle with a PID
# controlled start up using a coupled motor.
#
# Heat is supplied to the system by a volumetric heat source, and a second heat
# source is used to model a recuperator. The recuperator transfers heat from the
# turbine exhaust gas to the compressor outlet gas.
#
# Initially the fluid and heat structures are at rest at ambient conditions,
# and the shaft speed is zero.
# The transient is controlled as follows:
#   * 0   - 2000 s: Motor increases shaft speed to approx. 85,000 RPM by PID control
#   * 1000 - 8600 s: Power in main heat source increases from 0 - 104 kW
#   * 2000 - 200000 s: Torque supplied by turbine increases to steady state level
#                      as working fluid temperature increases. Torque supplied by
#                      the motor is ramped down to 0 N-m transitioning shaft control
#                      to the turbine at its rated speed of 96,000 RPM.


I_motor = 1.0

I_generator = 1.0
generator_torque_per_shaft_speed = -0.00025

motor_ramp_up_duration = 3605
motor_ramp_down_duration = 1800
post_motor_time = 2160000
t1 = ${motor_ramp_up_duration}
t2 = ${fparse t1 + motor_ramp_down_duration}
t3 = ${fparse t2 + post_motor_time}

D1 = 0.15
D2 = ${D1}
D3 = ${D1}
D4 = ${D1}
D5 = ${D1}
D6 = ${D1}
D7 = ${D1}
D8 = ${D1}

A1 = ${fparse 0.25 * pi * D1^2}
A2 = ${fparse 0.25 * pi * D2^2}
A3 = ${fparse 0.25 * pi * D3^2}
A4 = ${fparse 0.25 * pi * D4^2}
A5 = ${fparse 0.25 * pi * D5^2}
A6 = ${fparse 0.25 * pi * D6^2}
A7 = ${fparse 0.25 * pi * D7^2}
A8 = ${fparse 0.25 * pi * D8^2}

recuperator_width = 0.15

L1 = 5.0
L2 = ${L1}
L3 = ${fparse 2 * L1}
L4 = ${fparse 2 * L1}
L5 = ${L1}
L6 = ${L1}
L7 = ${fparse L1 + recuperator_width}
L8 = ${L1}

x1 = 0.0
x2 = ${fparse x1 + L1}
x3 = ${fparse x2 + L2}
x4 = ${x3}
x5 = ${fparse x4 - L4}
x6 = ${x5}
x7 = ${fparse x6 + L6}
x8 = ${fparse x7 + L7}

y1 = 0
y2 = ${y1}
y3 = ${y2}
y4 = ${fparse y3 - L3}
y5 = ${y4}
y6 = ${fparse y5 + L5}
y7 = ${y6}
y8 = ${y7}

x1_out = ${fparse x1 + L1 - 0.001}
x2_in = ${fparse x2 + 0.001}
y5_in = ${fparse y5 + 0.001}
x6_out = ${fparse x6 + L6 - 0.001}
x7_in = ${fparse x7 + 0.001}
y8_in = ${fparse y8 + 0.001}
y8_out = ${fparse y8 + L8 - 0.001}
hot_leg_in = ${y8_in}
hot_leg_out = ${y8_out}
cold_leg_in = ${fparse y3 - 0.001}
cold_leg_out = ${fparse y3 - (L3/2) - 0.001}

n_elems1 = 5
n_elems2 = ${n_elems1}
n_elems3 = ${fparse 2 * n_elems1}
n_elems4 = ${fparse 2 * n_elems1}
n_elems5 = ${n_elems1}
n_elems6 = ${n_elems1}
n_elems7 = ${n_elems1}
n_elems8 = ${n_elems1}

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

T_ambient = 300
p_ambient = 1e5

hs_power = 105750
[GlobalParams]
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
  scaling_factor_rhouV = 1e-2
  scaling_factor_rhovV = 1e-2
  scaling_factor_rhowV = 1e-2
  scaling_factor_rhoEV = 1e-5
  scaling_factor_temperature = 1e-2
  rdg_slope_reconstruction = none
[]

[FluidProperties]
  [fp_air]
    type = IdealGasFluidProperties
    emit_on_nan = none
  []
[]

[SolidProperties]
  [steel]
    type = ThermalFunctionSolidProperties
    rho = 8050
    k = 45
    cp = 466
  []
[]

[Closures]
  [closures]
    type = Closures1PhaseSimple
  []
[]

[Functions]

  ##########################
  # Motor
  ##########################


  # Functions for control logic that determines when to shut off the PID system
  [is_tripped_fn]
    type = ParsedFunction
    symbol_names = 'motor_torque turbine_torque'
    symbol_values = 'motor_torque turbine_torque'
    expression = 'turbine_torque > motor_torque'
  []
  [PID_tripped_constant_value]
    type = ConstantFunction
    value = 1
  []
  [PID_tripped_status_fn]
    type = ParsedFunction
    symbol_values = 'PID_trip_status'
    symbol_names = 'PID_trip_status'
    expression = 'PID_trip_status'
  []
  [time_fn]
    type = ParsedFunction
    expression = t
  []

  # Shutdown function which ramps down the motor once told by the control logic
  [motor_torque_fn_shutdown]
    type = ParsedFunction
    symbol_values = 'PID_trip_status time_trip'
    symbol_names = 'PID_trip_status time_trip'
    expression = 'if(PID_trip_status = 1, max(2.4 - (2.4 * ((t - time_trip) / 35000)),0.0), 1)'
  []

  # Generates motor power curve
  [motor_power_fn]
    type = ParsedFunction
    expression = 'torque * speed'
    symbol_names = 'torque speed'
    symbol_values = 'motor_torque shaft:omega'
  []

  ##########################
  # Generator
  ##########################

  # Generates generator torque curve
  [generator_torque_fn]
    type = ParsedFunction
    expression = 'slope * t'
    symbol_names = 'slope'
    symbol_values = '${generator_torque_per_shaft_speed}'
  []
  # Generates generator power curve
  [generator_power_fn]
    type = ParsedFunction
    expression = 'torque * speed'
    symbol_names = 'torque speed'
    symbol_values = 'generator_torque shaft:omega'
  []

  ##########################
  # Reactor
  ##########################

  # Ramps up reactor power when activated by control logic
  [power_fn]
    type = PiecewiseLinear
    x = '0 1000 8600'
    y = '0 0 ${hs_power}'
  []

  ##########################
  # Compressor
  ##########################

  # compressor pressure ratios
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

  # compressor efficiencies
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

  ##########################
  # Turbine
  ##########################

  # turbine pressure ratios
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

[Components]

  # system inlet pulling air from the open atmosphere
  [inlet]
    type = InletStagnationPressureTemperature1Phase
    input = 'pipe1:in'
    p0 = ${p_ambient}
    T0 = ${T_ambient}
  []

  # Inlet pipe
  [pipe1]
    type = FlowChannel1Phase
    position = '${x1} ${y1} 0'
    orientation = '1 0 0'
    length = ${L1}
    n_elems = ${n_elems1}
    A = ${A1}
  []

  # Compressor as defined in MAGNET PCU document (Guillen 2020)
  [compressor]
    type = ShaftConnectedCompressor1Phase
    position = '${x2} ${y2} 0'
    inlet = 'pipe1:out'
    outlet = 'pipe2:in'
    A_ref = ${A_ref_comp}
    volume = ${V_comp}

    omega_rated = ${speed_rated}
    mdot_rated = ${rated_mfr}
    c0_rated = ${c0_rated_comp}
    rho0_rated = ${rho0_rated_comp}

    # Determines which compression ratio curve and efficiency curve to use depending on ratio of speed/rated_speed
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

    use_scalar_variables = false
  []

  # Outlet pipe from the compressor
  [pipe2]
    type = FlowChannel1Phase
    position = '${x2} ${y2} 0'
    orientation = '1 0 0'
    length = ${L2}
    n_elems = ${n_elems2}
    A = ${A2}
  []

  # 90 degree connection between pipe 2 and 3
  [junction2_cold_leg]
    type = VolumeJunction1Phase
    connections = 'pipe2:out cold_leg:in'
    position = '${x3} ${y3} 0'
    volume = ${fparse A2*0.1}
    use_scalar_variables = false
  []

  # Cold leg of the recuperator
  [cold_leg]
    type = FlowChannel1Phase
    position = '${x3} ${y3} 0'
    orientation = '0 -1 0'
    length = ${fparse L3/2}
    n_elems = ${fparse n_elems3/2}
    A = ${A3}
  []

  # Recuperator which transfers heat from exhaust gas to reactor inlet gas to improve thermal efficency
  [recuperator]
    type = HeatStructureCylindrical
    orientation = '0 -1 0'
    position = '${x3} ${y3} 0'
    length = ${fparse L3/2}
    widths = ${recuperator_width}
    n_elems = ${fparse n_elems3/2}
    n_part_elems = 2
    names = recuperator
    solid_properties = steel
    solid_properties_T_ref = '300'
    inner_radius = ${D1}
  []
  # heat transfer from recuperator to cold leg
  [heat_transfer_cold_leg]
    type = HeatTransferFromHeatStructure1Phase
    flow_channel = cold_leg
    hs = recuperator
    hs_side = OUTER
    Hw = 10000
  []
  # heat transfer from hot leg to recuperator
  [heat_transfer_hot_leg]
    type = HeatTransferFromHeatStructure1Phase
    flow_channel = hot_leg
    hs = recuperator
    hs_side = INNER
    Hw = 10000
  []

  [junction_cold_leg_3]
    type = JunctionOneToOne1Phase
    connections = 'cold_leg:out pipe3:in'
  []
  [pipe3]
    type = FlowChannel1Phase
    position = '${x3} ${fparse y3 - (L3/2)} 0'
    orientation = '0 -1 0'
    length = ${fparse L3/2}
    n_elems = ${fparse n_elems3/2}
    A = ${A3}
  []
  # 90 degree connection between pipe 3 and 4
  [junction3_4]
    type = VolumeJunction1Phase
    connections = 'pipe3:out pipe4:in'
    position = '${x4} ${y4} 0'
    volume = ${fparse A3*0.1}
    use_scalar_variables = false
  []

  # Pipe through the "reactor core"
  [pipe4]
    type = FlowChannel1Phase
    position = '${x4} ${y4} 0'
    orientation = '-1 0 0'
    length = ${L4}
    n_elems = ${n_elems4}
    A = ${A4}
  []

  # "Reactor Core" and it's associated heat transfer to pipe 4
  [reactor]
    type = HeatStructureCylindrical
    orientation = '-1 0 0'
    position = '${x4} ${y4} 0'
    length = ${L4}
    widths = 0.15
    n_elems = ${n_elems4}
    n_part_elems = 2
    names = core
    solid_properties = steel
    solid_properties_T_ref = '300'
  []
  [total_power]
    type = TotalPower
    power = 0
  []
  [heat_generation]
    type = HeatSourceFromTotalPower
    power = total_power
    hs = reactor
    regions = core
  []
  [heat_transfer]
    type = HeatTransferFromHeatStructure1Phase
    flow_channel = pipe4
    hs = reactor
    hs_side = OUTER
    Hw = 10000
  []

  # 90 degree connection between pipe 4 and 5
  [junction4_5]
    type = VolumeJunction1Phase
    connections = 'pipe4:out pipe5:in'
    position = '${x5} ${y5} 0'
    volume = ${fparse A4*0.1}
    use_scalar_variables = false
  []

  # Pipe carrying hot gas back to the PCU
  [pipe5]
    type = FlowChannel1Phase
    position = '${x5} ${y5} 0'
    orientation = '0 1 0'
    length = ${L5}
    n_elems = ${n_elems5}
    A = ${A5}
  []

  # 90 degree connection between pipe 5 and 6
  [junction5_6]
    type = VolumeJunction1Phase
    connections = 'pipe5:out pipe6:in'
    position = '${x6} ${y6} 0'
    volume = ${fparse A5*0.1}
    use_scalar_variables = false
  []

  # Inlet pipe to the turbine
  [pipe6]
    type = FlowChannel1Phase
    position = '${x6} ${y6} 0'
    orientation = '1 0 0'
    length = ${L6}
    n_elems = ${n_elems6}
    A = ${A6}
  []

  # Turbine as defined in MAGNET PCU document (Guillen 2020) and (Wright 2006)
  [turbine]
    type = ShaftConnectedCompressor1Phase
    position = '${x7} ${y7} 0'
    inlet = 'pipe6:out'
    outlet = 'pipe7:in'
    A_ref = ${A_ref_turb}
    volume = ${V_turb}

    # A turbine is treated as an "inverse" compressor, this value determines if component is to be treated as turbine or compressor
    # If treat_as_turbine is omitted, code automatically assumes it is a compressor
    treat_as_turbine = true

    omega_rated = ${speed_rated}
    mdot_rated = ${rated_mfr}
    c0_rated = ${c0_rated_comp}
    rho0_rated = ${rho0_rated_comp}

    # Determines which compression ratio curve and efficiency curve to use depending on ratio of speed/rated_speed
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

    use_scalar_variables = false
  []

  # Outlet pipe from turbine
  [pipe7]
    type = FlowChannel1Phase
    position = '${x7} ${y7} 0'
    orientation = '1 0 0'
    length = ${L7}
    n_elems = ${n_elems7}
    A = ${A7}
  []

  # 90 degree connection between pipe 7 and 8
  [junction7_hot_leg]
    type = VolumeJunction1Phase
    connections = 'pipe7:out hot_leg:in'
    position = '${x8} ${y8} 0'
    volume = ${fparse A7*0.1}
    use_scalar_variables = false
  []

  # Hot leg of the recuperator
  [hot_leg]
    type = FlowChannel1Phase
    position = '${x8} ${y8} 0'
    orientation = '0 1 0'
    length = ${L8}
    n_elems = ${n_elems8}
    A = ${A8}
  []

  # System outlet dumping exhaust gas to the atmosphere
  [outlet]
    type = Outlet1Phase
    input = 'hot_leg:out'
    p = ${p_ambient}
  []

  # Roatating shaft connecting motor, compressor, turbine, and generator
  [shaft]
    type = Shaft
    connected_components = 'motor compressor turbine generator'
    initial_speed = ${speed_initial}
  []

  # 3-Phase electircal motor used for system start-up, controlled by PID
  [motor]
    type = ShaftConnectedMotor
    inertia = ${I_motor}
    torque = 0 # controlled
  []

  # Electric generator supplying power to the grid
  [generator]
    type = ShaftConnectedMotor
    inertia = ${I_generator}
    torque = generator_torque_fn
  []
[]


# Control logics which govern startup of the motor, startup of the "reactor core", and shutdown of the motor
[ControlLogic]

  # Sets desired shaft speed to be reached by motor NOTE: SHOULD BE SET LOWER THAN RATED TURBINE RPM
  [set_point]
    type = GetFunctionValueControl
    function = ${fparse speed_rated_rpm - 9000}
  []

  # PID with gains determined by iterative process NOTE: Gain values are system specific
  [initial_motor_PID]
    type = PIDControl
    set_point = set_point:value
    input = shaft_RPM
    initial_value = 0
    K_p = 0.0011
    K_i = 0.00000004
    K_d = 0
  []

  # Determines when the PID system should be running and when it should begin the shutdown cycle. If needed: PID output, else: shutdown function
  [logic]
    type = ParsedFunctionControl
    function = 'if(motor+0.5 > turb, PID, shutdown_fn)'
    symbol_names = 'motor turb PID shutdown_fn'
    symbol_values = 'motor_torque turbine_torque initial_motor_PID:output motor_torque_fn_shutdown'
  []

  # Takes the output generated in [logic] and applies it to the motor torque
  [motor_PID]
    type = SetComponentRealValueControl
    component = motor
    parameter = torque
    value = logic:value
  []
  # Determines when to turn on heat source
  [power_logic]
    type = ParsedFunctionControl
    function = 'power_fn'
    symbol_names = 'power_fn'
    symbol_values = 'power_fn'
  []
  # Applies heat source to the total_power block
  [power_applied]
    type = SetComponentRealValueControl
    component = total_power
    parameter = power
    value = power_logic:value
  []
[]

[Controls]

  # Enables set_PID_tripped
  [PID_trip_status]
    type = ConditionalFunctionEnableControl
    conditional_function = is_tripped_fn
    enable_objects = 'AuxScalarKernels::PID_trip_status_aux'
    execute_on = 'TIMESTEP_END'
  []

  # Enables set_time_PID
  [time_PID]
    type = ConditionalFunctionEnableControl
    conditional_function = PID_tripped_status_fn
    disable_objects = 'AuxScalarKernels::time_trip_aux'
    execute_on = 'TIMESTEP_END'
  []
[]

[AuxVariables]

  # Creates a variable that will later be set to the time when tau_turbine > tau_motor
  [time_trip]
    order = FIRST
    family = SCALAR
  []

  # Creates variable which indicates if tau_turbine > tau_motor....... If tau_motor > tau_turbine, 0, else 1
  [PID_trip_status]
    order = FIRST
    family = SCALAR
    initial_condition = 0
  []
[]

[AuxScalarKernels]

  # Creates variable from time_fn which indicates when tau_turbine > tau_motor
  [time_trip_aux]
    type = FunctionScalarAux
    function = time_fn
    variable = time_trip
    execute_on = 'TIMESTEP_END'
  []

  # Overwrites variable PID_trip_status to the value from PID_tripped_constant_value (changes 0 to 1)
  [PID_trip_status_aux]
    type = FunctionScalarAux
    function = PID_tripped_constant_value
    variable = PID_trip_status
    execute_on = 'TIMESTEP_END'
    enable = false
  []
[]


[Postprocessors]

  # Indicates when tau_turbine > tau_motor
  [trip_time]
    type = ScalarVariable
    variable = time_trip
    execute_on = 'TIMESTEP_END'
  []

  ##########################
  # Motor
  ##########################

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
  []

  ##########################
  # generator
  ##########################

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
  []

  ##########################
  # Shaft
  ##########################

  # Speed in rad/s
  [shaft_speed]
    type = ScalarVariable
    variable = 'shaft:omega'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  # speed in RPM
  [shaft_RPM]
    type = ParsedPostprocessor
    pp_names = 'shaft_speed'
    expression = '(shaft_speed * 60) /( 2 * ${fparse pi})'
    execute_on = 'INITIAL TIMESTEP_END'
  []

  ##########################
  # Compressor
  ##########################
  [comp_dissipation_torque]
    type = ElementAverageValue
    variable = dissipation_torque
    block = 'compressor'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [comp_isentropic_torque]
    type = ElementAverageValue
    variable = isentropic_torque
    block = 'compressor'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [comp_friction_torque]
    type = ElementAverageValue
    variable = friction_torque
    block = 'compressor'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [compressor_torque]
    type = ParsedPostprocessor
    pp_names = 'comp_dissipation_torque comp_isentropic_torque comp_friction_torque'
    expression = 'comp_dissipation_torque + comp_isentropic_torque + comp_friction_torque'
  []
  [p_in_comp]
    type = PointValue
    variable = p
    point = '${x1_out} ${y1} 0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [p_out_comp]
    type = PointValue
    variable = p
    point = '${x2_in} ${y2} 0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [p_ratio_comp]
    type = ParsedPostprocessor
    pp_names = 'p_in_comp p_out_comp'
    expression = 'p_out_comp / p_in_comp'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [T_in_comp]
    type = PointValue
    variable = T
    point = '${x1_out} ${y1} 0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [T_out_comp]
    type = PointValue
    variable = T
    point = '${x2_in} ${y2} 0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [T_ratio_comp]
    type = ParsedPostprocessor
    pp_names = 'T_in_comp T_out_comp'
    expression = '(T_out_comp - T_in_comp) / T_out_comp'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [mfr_comp]
    type = ADFlowJunctionFlux1Phase
    boundary = pipe1:out
    connection_index = 0
    equation = mass
    junction = compressor
  []

  ##########################
  # turbine
  ##########################

  [turb_dissipation_torque]
    type = ElementAverageValue
    variable = dissipation_torque
    block = 'turbine'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [turb_isentropic_torque]
    type = ElementAverageValue
    variable = isentropic_torque
    block = 'turbine'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [turb_friction_torque]
    type = ElementAverageValue
    variable = friction_torque
    block = 'turbine'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [turbine_torque]
    type = ParsedPostprocessor
    pp_names = 'turb_dissipation_torque turb_isentropic_torque turb_friction_torque'
    expression = 'turb_dissipation_torque + turb_isentropic_torque + turb_friction_torque'
  []
  [p_in_turb]
    type = PointValue
    variable = p
    point = '${x6_out} ${y6} 0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [p_out_turb]
    type = PointValue
    variable = p
    point = '${x7_in} ${y7} 0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [p_ratio_turb]
    type = ParsedPostprocessor
    pp_names = 'p_in_turb p_out_turb'
    expression = 'p_in_turb / p_out_turb'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [T_in_turb]
    type = PointValue
    variable = T
    point = '${x6_out} ${y6} 0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [T_out_turb]
    type = PointValue
    variable = T
    point = '${x7_in} ${y7} 0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [mfr_turb]
    type = ADFlowJunctionFlux1Phase
    boundary = pipe6:out
    connection_index = 0
    equation = mass
    junction = turbine
  []

  ##########################
  # Recuperator
  ##########################

  [cold_leg_in]
    type = PointValue
    variable = T
    point = '${x3} ${cold_leg_in} 0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [cold_leg_out]
    type = PointValue
    variable = T
    point = '${x3} ${cold_leg_out} 0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [hot_leg_in]
    type = PointValue
    variable = T
    point = '${x8} ${hot_leg_in} 0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [hot_leg_out]
    type = PointValue
    variable = T
    point = '${x8} ${hot_leg_out} 0'
    execute_on = 'INITIAL TIMESTEP_END'
  []

  ##########################
  # Reactor
  ##########################

  [reactor_inlet]
    type = PointValue
    variable = T
    point = '${x4} ${y4} 0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [reactor_outlet]
    type = PointValue
    variable = T
    point = '${x5} ${y5_in} 0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  end_time = ${t3}
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 0.01
    growth_factor = 1.1
    cutback_factor = 0.9
  []
  dtmin = 1e-5
  dtmax = 1000
  steady_state_detection = true
  steady_state_start_time = 200000

  solve_type = NEWTON
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
  nl_max_its = 15

  l_tol = 1e-4
  l_max_its = 10

  petsc_options_iname  = '-pc_type'
  petsc_options_value  = ' lu     '
[]

[Outputs]
  [e]
    type = Exodus
    file_base = 'recuperated_brayton_cycle_out'
  []
  [csv]
    type = CSV
    file_base = 'recuperated_brayton_cycle'
    execute_vector_postprocessors_on = 'INITIAL'
  []
  [console]
    type = Console
    show = 'shaft_speed p_ratio_comp p_ratio_turb pressure_ratio pressure_ratio'
  []
[]
