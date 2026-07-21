T_in = 667.15
outlet_pressure = 1.01325e5
assembly_power = 4000.0
volume_flow_rate = 0.002090242
rho_in = '${fparse 1004.23 - 0.21390 * T_in - 1.1046e-5 * T_in * T_in}'
mass_flow_rate = '${fparse volume_flow_rate * rho_in}'
core_length = 1.145
pin_pitch = 0.027813
pin_diameter = 0.024638
duct_inner_ftf = 0.0769620
n_pins = 7
flow_area = ${fparse sqrt(3)/2 * duct_inner_ftf ^2 - n_pins * pi * pin_diameter^2 / 4}
mass_flux_in = '${fparse mass_flow_rate / flow_area}'
axial_shape_scale = 1.33449883449883

[TriSubChannelMesh]
  [assembly]
    type = SCMTriAssemblyMeshGenerator
    nrings = 2
    n_cells = 20
    heated_length = ${core_length}
    flat_to_flat = ${duct_inner_ftf}
    pin_diameter = ${pin_diameter}
    pitch = ${pin_pitch}
    dwire = 0.0
    hwire = 0.0
    spacer_z = '0.0'
    spacer_k = '0.0'
  []
[]

[FluidProperties]
  [sodium]
    type = PBSodiumFluidProperties
  []
[]

[Functions]
  [axial_heat_rate]
    type = PiecewiseConstant
    axis = z
    x = '0.000 0.118 0.641 0.692 1.027'
    y = '0.0 ${axial_shape_scale} 0.0 ${axial_shape_scale} 0.0'
    direction = left
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
  gravity = counter_flow
  friction_closure = uctd_friction
  mixing_closure = kim_chung_mixing
  pin_HTC_closure = kazimi_carelli_htc
  P_tol = 1.0e-8
  T_tol = 1.0e-8
  full_output = true
  verbose_subchannel = true
[]

[SCMClosures]
  [uctd_friction]
    type = SCMFrictionUpdatedChengTodreas
  []
  [kim_chung_mixing]
    type = SCMMixingKimAndChung
  []
  [kazimi_carelli_htc]
    type = SCMHTCKazimiCarelli
  []
[]

[ICs]
  [q_prime_ic]
    type = SCMTriPowerIC
    variable = q_prime
    power = ${assembly_power}
    filename = "pin_power_profile7.txt"
    axial_heat_rate = axial_heat_rate
  []
  [Dpin_ic]
    type = ConstantIC
    variable = Dpin
    value = ${pin_diameter}
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
  [DP_ic]
    type = ConstantIC
    variable = DP
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
  [inlet_temperature]
    type = ConstantAux
    variable = T
    boundary = inlet
    value = ${T_in}
    execute_on = timestep_begin
  []
  [inlet_mass_flow]
    type = SCMMassFlowRateAux
    variable = mdot
    boundary = inlet
    area = S
    mass_flux = ${mass_flux_in}
    execute_on = timestep_begin
  []
[]

[Postprocessors]
  [total_pin_power]
    type = ElementIntegralVariablePostprocessor
    variable = q_prime
    block = fuel_pins
  []
  [outlet_mean_temperature]
    type = SCMPlanarMean
    variable = T
    height = ${core_length}
  []
  [assembly_pressure_drop]
    type = SubChannelDelta
    variable = P
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = false
  exodus = false
[]
