T_in = 359.15
# [1e+6 kg/m^2-hour] turns into kg/m^2-sec
mass_flux_in = '${fparse 1e+6 * 17.00 / 3600.}'
P_out = 4.923e6 # Pa
heated_length = 1.0
# Radial weights are 1 and 2, so the pins should receive 1/3 and 2/3 of this power
power = 30000.0 # W

# Two pins at x = -pitch/2 and x = +pitch/2
[QuadSubChannelMesh]
  [subchannel]
    type = SCMQuadAssemblyMeshGenerator
    nx = 3
    ny = 2
    n_cells = 10
    pitch = 0.0126
    pin_diameter = 0.00950
    side_gap = 0.00095
    heated_length = ${heated_length}
  []
[]

[Functions]
  # A sine shape on the pin at x < 0 and a linear shape on the pin at x > 0. Neither is
  # normalized to the heated length.
  [axial_heat_rate]
    type = ParsedFunction
    expression = 'if(x < 0, sin(pi*z/L), 5*z/L)'
    symbol_names = 'L'
    symbol_values = '${heated_length}'
  []
[]

[AuxVariables]
  [q_prime_aux]
    block = fuel_pins
  []
  [q_prime_IC_pin0]
    block = fuel_pins
  []
  [q_prime_IC_pin1]
    block = fuel_pins
  []
  [q_prime_aux_pin0]
    block = fuel_pins
  []
  [q_prime_aux_pin1]
    block = fuel_pins
  []
[]

[FluidProperties]
  [water]
    type = Water97FluidProperties
  []
[]

[SubChannel]
  type = QuadSubChannel1PhaseProblem
  fp = water
  n_blocks = 1
  compute_density = true
  compute_viscosity = true
  compute_power = true
  P_out = ${P_out}
  friction_closure = 'MATRA'
  mixing_closure = 'constant_beta'
  pin_HTC_closure = 'Dittus-Boelter'
[]

[SCMClosures]
  [MATRA]
    type = SCMFrictionMATRA
  []
  [constant_beta]
    type = SCMMixingConstantBeta
    beta = 0.006
    CT = 2.6
  []
  [Dittus-Boelter]
    type = SCMHTCDittusBoelter
  []
[]

[ICs]
  [q_prime_IC]
    type = SCMQuadPowerIC
    variable = q_prime
    power = ${power}
    filename = 'two_pin_power_profile.txt'
    axial_heat_rate = axial_heat_rate
  []
  [T_ic]
    type = ConstantIC
    variable = T
    value = ${T_in}
  []
  [P_ic]
    type = ConstantIC
    variable = P
    value = 0.0
  []
  [Viscosity_ic]
    type = ViscosityIC
    variable = mu
    p = ${P_out}
    T = T
    fp = water
  []
  [rho_ic]
    type = RhoFromPressureTemperatureIC
    variable = rho
    p = ${P_out}
    T = T
    fp = water
  []
  [h_ic]
    type = SpecificEnthalpyFromPressureTemperatureIC
    variable = h
    p = ${P_out}
    T = T
    fp = water
  []
  [mdot_ic]
    type = ConstantIC
    variable = mdot
    value = 0.0
  []
[]

[AuxKernels]
  [q_prime_AUX]
    type = SCMQuadPowerAux
    variable = q_prime_aux
    power = ${power}
    filename = 'two_pin_power_profile.txt'
    axial_heat_rate = axial_heat_rate
    execute_on = 'initial'
  []
  # Split q_prime by pin so that each pin's power can be integrated separately
  [IC_pin0]
    type = ParsedAux
    variable = q_prime_IC_pin0
    coupled_variables = 'q_prime'
    expression = 'if(x < 0, q_prime, 0)'
    use_xyzt = true
  []
  [IC_pin1]
    type = ParsedAux
    variable = q_prime_IC_pin1
    coupled_variables = 'q_prime'
    expression = 'if(x > 0, q_prime, 0)'
    use_xyzt = true
  []
  [aux_pin0]
    type = ParsedAux
    variable = q_prime_aux_pin0
    coupled_variables = 'q_prime_aux'
    expression = 'if(x < 0, q_prime_aux, 0)'
    use_xyzt = true
  []
  [aux_pin1]
    type = ParsedAux
    variable = q_prime_aux_pin1
    coupled_variables = 'q_prime_aux'
    expression = 'if(x > 0, q_prime_aux, 0)'
    use_xyzt = true
  []
  [T_in_bc]
    type = ConstantAux
    variable = T
    boundary = inlet
    value = ${T_in}
    execute_on = 'timestep_begin'
  []
  [mdot_in_bc]
    type = SCMMassFlowRateAux
    variable = mdot
    boundary = inlet
    area = S
    mass_flux = ${mass_flux_in}
    execute_on = 'timestep_begin'
  []
[]

[Postprocessors]
  [pin0_power_IC]
    type = ElementIntegralVariablePostprocessor
    variable = q_prime_IC_pin0
    block = fuel_pins
  []
  [pin1_power_IC]
    type = ElementIntegralVariablePostprocessor
    variable = q_prime_IC_pin1
    block = fuel_pins
  []
  [pin0_power_aux]
    type = ElementIntegralVariablePostprocessor
    variable = q_prime_aux_pin0
    block = fuel_pins
  []
  [pin1_power_aux]
    type = ElementIntegralVariablePostprocessor
    variable = q_prime_aux_pin1
    block = fuel_pins
  []
  [total_power_TH]
    type = SCMTHPowerPostprocessor
  []
[]

[VectorPostprocessors]
  # Axial q_prime profile of both pins
  [q_prime_profiles]
    type = NodalValueSampler
    variable = 'q_prime q_prime_aux'
    block = fuel_pins
    sort_by = id
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
