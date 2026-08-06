# SCMBulkOutletTemperature analytic solve test
#
# This is a real steady subchannel solve. It exercises:
#
#   1. The inlet mass-flow boundary condition.
#   2. The subchannel mass, momentum, and enthalpy solution.
#   3. Crossflow and turbulent enthalpy mixing.
#   4. Mass-flow weighting of the solved outlet enthalpies.
#   5. Conversion of the bulk outlet enthalpy back to temperature.
#
# -----------------------------------------------------------------------------
# Analytic solution
# -----------------------------------------------------------------------------
#
# The two inlet channel areas are
#
#   S_left  = 1.0e-3 m^2
#   S_right = 2.0e-3 m^2
#
# and the common inlet mass flux is
#
#   G = 1000 kg/(m^2 s).
#
# Therefore, the inlet mass-flow rates are
#
#   mdot_left  = G S_left  = 1 kg/s
#   mdot_right = G S_right = 2 kg/s.
#
# The inlet temperatures are
#
#   T_left  = 300 K
#   T_right = 600 K.
#
# SimpleFluidProperties is configured with
#
#   cv = 1000 J/(kg K)
#   porepressure_coefficient = 0.
#
# Consequently,
#
#   h(p,T) = cv T
#
# exactly, independent of pressure. Thus,
#
#   h_left  = 1000(300) = 300000 J/kg
#   h_right = 1000(600) = 600000 J/kg.
#
# There is no applied heat:
#
#   q_prime = 0.
#
# Conservation of total mass and total enthalpy gives
#
#   mdot_bulk,out = 1 + 2 = 3 kg/s
#
# and
#
#   Hdot_out = Hdot_in
#            = (1)(300000) + (2)(600000)
#            = 1500000 W.
#
# Therefore,
#
#   h_bulk,out = Hdot_out / mdot_bulk,out
#              = 1500000 / 3
#              = 500000 J/kg,
#
# and the expected postprocessor value is
#
#   T_bulk,out = h_bulk,out / cv
#              = 500000 / 1000
#              = 500 K.
#
# Crossflow and turbulent mixing can redistribute mass and enthalpy between
# the two channels, but they cannot change the assembly-integrated mass or
# enthalpy. The expected bulk temperature therefore remains 500 K.
#
# An incorrect unweighted average of the two inlet temperatures would give
#
#   (300 + 600) / 2 = 450 K,
#
# so this test distinguishes mass-flow weighting from arithmetic averaging.

mass_flux_in = 1000 # kg/(m^2 s)
P_out = 1e5 # Pa

[QuadSubChannelMesh]
  [sub_channel]
    type = SCMQuadAssemblyMeshGenerator

    # This geometry produces two subchannels whose x coordinates have
    # opposite signs, allowing the ParsedFunctions below to distinguish them.
    nx = 2
    ny = 1

    # Only a small axial mesh is needed because the expected assembly-integrated
    # result follows directly from conservation.
    n_cells = 10

    pitch = 0.0126
    pin_diameter = 0.00950
    side_gap = 0.00095
    heated_length = 1.0

    # No spacer-grid form losses.
    spacer_z = '0.0'
    spacer_k = '0.0'
  []
[]

[Functions]
  [area_fn]
    type = ParsedFunction

    # Left channel:  1e-3 m^2
    # Right channel: 2e-3 m^2
    #
    # With mass_flux_in = 1000 kg/(m^2 s), these areas produce inlet
    # mass-flow rates of 1 kg/s and 2 kg/s, respectively.
    expression = 'if(x > 0.0, 2e-3, 1e-3)'
  []

  [temperature_fn]
    type = ParsedFunction

    # Left channel:  300 K
    # Right channel: 600 K
    expression = 'if(x > 0.0, 600.0, 300.0)'
  []
[]

[FluidProperties]
  [simple_fp]
    type = SimpleFluidProperties

    # Setting cp = cv also makes h/cp the exact initial guess used internally
    # by the T_from_p_h Newton inversion.
    cv = 1000
    cp = 1000

    density0 = 1000
    thermal_expansion = 2.14e-4
    bulk_modulus = 2e9
    viscosity = 1e-3

    # Test-only analytic simplification:
    #
    #   h = e + coefficient*p/rho
    #   e = cv*T
    #
    # Setting the coefficient to zero gives h = cv*T exactly. The momentum
    # solve may still generate pressure variations, but those variations do
    # not affect the analytic enthalpy-temperature relationship.
    porepressure_coefficient = 0
  []
[]

[SubChannel]
  type = QuadSubChannel1PhaseProblem
  fp = simple_fp
  n_blocks = 1

  # Absolute outlet pressure. Internally, the subchannel pressure variable is
  # a pressure difference relative to this value.
  P_out = ${P_out}

  # Solve the property and energy equations rather than using a NoSolveProblem.
  compute_density = true
  compute_viscosity = true
  compute_power = true

  P_tol = 1e-8
  T_tol = 1e-8

  friction_closure = 'MATRA'
  mixing_closure = 'constant_beta'

  full_output = true
[]

[SCMClosures]
  [MATRA]
    type = SCMFrictionMATRA
  []

  [constant_beta]
    type = SCMMixingConstantBeta

    # Keep the mixing model active. It will alter the individual outlet
    # temperatures, but it must preserve the total enthalpy flow.
    beta = 0.006
    CT = 2.0
  []
[]

[ICs]
  [area_ic]
    type = FunctionIC
    variable = S
    function = area_fn
  []

  [temperature_ic]
    type = FunctionIC
    variable = T
    function = temperature_fn
  []

  [wetted_perimeter_ic]
    type = ConstantIC
    variable = w_perim
    value = 0.34188034
  []

  [linear_heat_rate_ic]
    type = ConstantIC
    variable = q_prime

    # No external energy is added. Therefore, the total outlet enthalpy flow
    # must equal the total inlet enthalpy flow.
    value = 0
  []

  [pressure_drop_ic]
    type = ConstantIC
    variable = P
    value = 0
  []

  [axial_pressure_drop_ic]
    type = ConstantIC
    variable = DP
    value = 0
  []

  [viscosity_ic]
    type = ViscosityIC
    variable = mu
    p = ${P_out}
    T = T
    fp = simple_fp
  []

  [density_ic]
    type = RhoFromPressureTemperatureIC
    variable = rho
    p = ${P_out}
    T = T
    fp = simple_fp
  []

  [enthalpy_ic]
    type = SpecificEnthalpyFromPressureTemperatureIC
    variable = h
    p = ${P_out}
    T = T
    fp = simple_fp
  []

  [mass_flow_ic]
    type = ConstantIC
    variable = mdot
    value = 0
  []
[]

[AuxKernels]
  [inlet_mass_flow]
    type = SCMMassFlowRateAux
    variable = mdot
    boundary = inlet
    area = S
    mass_flux = ${mass_flux_in}
    execute_on = 'timestep_begin'
  []
[]

[Postprocessors]
  [outlet_pressure]
    type = Receiver

    # This is the same absolute outlet pressure supplied to the subchannel
    # problem. Using a Receiver follows the intended interface for pressures
    # that may eventually be transferred from another application.
    default = ${P_out}
    execute_on = 'initial timestep_end'
  []

  [bulk_temperature]
    type = SCMBulkOutletTemperature
    pressure = outlet_pressure
    fp = simple_fp
    execute_on = 'timestep_end'
  []

  # The following point values are not needed for the actual regression check,
  # but can be useful while initially debugging the test. They may be removed
  # after confirming the solution.
  [outlet_temperature_left]
    type = SubChannelPointValue
    variable = T
    index = 0
    height = 1.0
    execute_on = 'timestep_end'
  []

  [outlet_temperature_right]
    type = SubChannelPointValue
    variable = T
    index = 1
    height = 1.0
    execute_on = 'timestep_end'
  []

  [outlet_mdot_left]
    type = SubChannelPointValue
    variable = mdot
    index = 0
    height = 1.0
    execute_on = 'timestep_end'
  []

  [outlet_mdot_right]
    type = SubChannelPointValue
    variable = mdot
    index = 1
    height = 1.0
    execute_on = 'timestep_end'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true

  # Avoid an initial CSV row containing postprocessor values from before the
  # steady solve. The regression result should contain the solved state only.
  execute_on = 'timestep_end'
[]
