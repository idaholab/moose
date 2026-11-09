T_in = 667.15
outlet_pressure = 1.01325e5 # Pa
volume_flow_rate = 0.002090242 # m^3/s
rho_in = '${fparse 1004.23 - 0.21390 * T_in - 1.1046e-5 * T_in * T_in}'
duct_inner_ftf = 0.0769620
pin_diameter = 0.024638
flow_area = '${fparse sqrt(3)/2 * duct_inner_ftf^2 - 7 * pi * pin_diameter^2 / 4}'
mass_flux_in = '${fparse volume_flow_rate * rho_in / flow_area}'
heated_length = 1.0
pin_pitch = 0.027813
# Uniform radial weights, so each of the 7 pins should receive 1/7 of this power
power = 7000.0 # W

# Seven pins: the center pin at the origin and six outer pins one pitch away from it
[TriSubChannelMesh]
  [assembly]
    type = SCMTriAssemblyMeshGenerator
    nrings = 2
    n_cells = 10
    heated_length = ${heated_length}
    flat_to_flat = ${duct_inner_ftf}
    pin_diameter = ${pin_diameter}
    pitch = ${pin_pitch}
    dwire = 0.0
    hwire = 0.0
  []
[]

[Functions]
  # A sine shape on the center pin and a linear shape on the outer pins. Neither is normalized
  # to the heated length. The break at half a pitch from the center lies between the pins.
  [axial_heat_rate]
    type = ParsedFunction
    expression = 'if(sqrt(x*x + y*y) < 0.5*p, sin(pi*z/L), 5*z/L)'
    symbol_names = 'L p'
    symbol_values = '${heated_length} ${pin_pitch}'
  []
[]

[AuxVariables]
  [q_prime_aux]
    block = fuel_pins
  []
  [q_prime_IC_center]
    block = fuel_pins
  []
  [q_prime_IC_outer]
    block = fuel_pins
  []
  [q_prime_aux_center]
    block = fuel_pins
  []
  [q_prime_aux_outer]
    block = fuel_pins
  []
[]

[FluidProperties]
  [sodium]
    type = PBSodiumFluidProperties
  []
[]

[SubChannel]
  type = TriSubChannel1PhaseProblem
  fp = sodium
  n_blocks = 1
  P_out = ${outlet_pressure}
  compute_density = true
  compute_viscosity = true
  compute_power = true
  implicit = true
  segregated = false
  interpolation_scheme = upwind
  friction_closure = uctd_friction
  mixing_closure = kim_chung_mixing
  pin_HTC_closure = kazimi_carelli_htc
  P_tol = 1.0e-8
  T_tol = 1.0e-8
  rtol = 1.0e-10
  atol = 1.0e-10
[]

[SCMClosures]
  [uctd_friction]
    type = SCMFrictionUpgradedChengTodreas
  []
  [kim_chung_mixing]
    type = SCMMixingKimAndChung
  []
  [kazimi_carelli_htc]
    type = SCMHTCKazimiCarelli
  []
[]

[ICs]
  [q_prime_IC]
    type = SCMTriPowerIC
    variable = q_prime
    power = ${power}
    filename = 'seven_pin_power_profile.txt'
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
  [mu_ic]
    type = ViscosityIC
    variable = mu
    p = ${outlet_pressure}
    T = T
    fp = sodium
  []
  [rho_ic]
    type = RhoFromPressureTemperatureIC
    variable = rho
    p = ${outlet_pressure}
    T = T
    fp = sodium
  []
  [h_ic]
    type = SpecificEnthalpyFromPressureTemperatureIC
    variable = h
    p = ${outlet_pressure}
    T = T
    fp = sodium
  []
  [mdot_ic]
    type = ConstantIC
    variable = mdot
    value = 0.0
  []
[]

[AuxKernels]
  [q_prime_AUX]
    type = SCMTriPowerAux
    variable = q_prime_aux
    power = ${power}
    filename = 'seven_pin_power_profile.txt'
    axial_heat_rate = axial_heat_rate
    execute_on = 'initial'
  []
  # Split q_prime between the center pin and the outer pins so each power is integrated separately
  [IC_center]
    type = ParsedAux
    variable = q_prime_IC_center
    coupled_variables = 'q_prime'
    expression = 'if(sqrt(x*x + y*y) < 0.5*${pin_pitch}, q_prime, 0)'
    use_xyzt = true
  []
  [IC_outer]
    type = ParsedAux
    variable = q_prime_IC_outer
    coupled_variables = 'q_prime'
    expression = 'if(sqrt(x*x + y*y) > 0.5*${pin_pitch}, q_prime, 0)'
    use_xyzt = true
  []
  [aux_center]
    type = ParsedAux
    variable = q_prime_aux_center
    coupled_variables = 'q_prime_aux'
    expression = 'if(sqrt(x*x + y*y) < 0.5*${pin_pitch}, q_prime_aux, 0)'
    use_xyzt = true
  []
  [aux_outer]
    type = ParsedAux
    variable = q_prime_aux_outer
    coupled_variables = 'q_prime_aux'
    expression = 'if(sqrt(x*x + y*y) > 0.5*${pin_pitch}, q_prime_aux, 0)'
    use_xyzt = true
  []
  [inlet_temperature]
    type = ConstantAux
    variable = T
    boundary = inlet
    value = ${T_in}
    execute_on = 'timestep_begin'
  []
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
  [center_pin_power_IC]
    type = ElementIntegralVariablePostprocessor
    variable = q_prime_IC_center
    block = fuel_pins
  []
  [outer_pins_power_IC]
    type = ElementIntegralVariablePostprocessor
    variable = q_prime_IC_outer
    block = fuel_pins
  []
  [center_pin_power_aux]
    type = ElementIntegralVariablePostprocessor
    variable = q_prime_aux_center
    block = fuel_pins
  []
  [outer_pins_power_aux]
    type = ElementIntegralVariablePostprocessor
    variable = q_prime_aux_outer
    block = fuel_pins
  []
  [total_power_TH]
    type = SCMTHPowerPostprocessor
  []
[]

[VectorPostprocessors]
  # Axial q_prime profile of all pins
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
