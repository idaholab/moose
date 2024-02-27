# M. Avramova et. all 2012,
# OECD/NRC Benchmark Based on NUPEC PWR
# Sub-channel and Bundle Tests (PSBT). Volume III: Departure from Nucleate Boiling
T_in = 502.35
# [1e+6 kg/m^2-hour] turns into kg/m^2-sec
mass_flux_in = '${fparse 1e+6 * 16.95 / 3600.}'
P_out = 14.72e6 # Pa
[QuadSubChannelMesh]
  [sub_channel]
    type = QuadSubChannelMeshGenerator
    nx = 6
    ny = 6
    n_cells = 50
    pitch = 0.0126
    rod_diameter = 0.00950
    gap = 0.00095
    heated_length = 3.658
    spacer_z = '0.0 0.229 0.457 0.686 0.914 1.143 1.372 1.600 1.829 2.057 2.286 2.515 2.743 2.972 3.200 3.429'
    spacer_k = '0.7 0.4 1.0 0.4 1.0 0.4 1.0 0.4 1.0 0.4 1.0 0.4 1.0 0.4 1.0 0.4'
  []

  [fuel_pins]
    type = QuadPinMeshGenerator
    input = sub_channel
    nx = 6
    ny = 6
    n_cells = 50
    pitch = 0.0126
    heated_length = 3.658
  []
[]

[AuxVariables]
  [mdot]
    block = sub_channel
  []
  [SumWij]
    block = sub_channel
  []
  [P]
    block = sub_channel
  []
  [DP]
    block = sub_channel
  []
  [h]
    block = sub_channel
  []
  [T]
    block = sub_channel
  []
  [Tpin]
    block = fuel_pins
  []
  [Dpin]
    block = fuel_pins
  []
  [rho]
    block = sub_channel
  []
  [mu]
    block = sub_channel
  []
  [S]
    block = sub_channel
  []
  [w_perim]
    block = sub_channel
  []
  [q_prime]
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
  beta = 0.006
  CT = 2.6
  compute_density = true
  compute_viscosity = true
  compute_power = true
  P_out = ${P_out}
  implicit = true
  segregated = false
  staggered_pressure = false
  monolithic_thermal = false
[]

[ICs]
  [S_IC]
    type = QuadFlowAreaIC
    variable = S
  []

  [w_perim_IC]
    type = QuadWettedPerimIC
    variable = w_perim
  []

  [q_prime_IC]
    type = QuadPowerIC
    variable = q_prime
    power = 3.23e6 # W
    filename = "power_profile.txt"
  []

  [T_ic]
    type = ConstantIC
    variable = T
    value = ${T_in}
  []

  [Dpin_ic]
    type = ConstantIC
    variable = Dpin
    value = 0.00950
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
  [T_in_bc]
    type = ConstantAux
    variable = T
    boundary = inlet
    value = ${T_in}
    execute_on = 'timestep_begin'
  []
  [mdot_in_bc]
    type = MassFlowRateAux
    variable = mdot
    boundary = inlet
    area = S
    mass_flux = ${mass_flux_in}
    execute_on = 'timestep_begin'
  []
[]

[Outputs]
  exodus = true
  [Temp_Out_MATRIX]
    type = QuadSubChannelNormalSliceValues
    variable = T
    execute_on = final
    file_base = "Temp_Out.txt"
    height = 3.658
  []
  [mdot_Out_MATRIX]
    type = QuadSubChannelNormalSliceValues
    variable = mdot
    execute_on = final
    file_base = "mdot_Out.txt"
    height = 3.658
  []
  [mdot_In_MATRIX]
    type = QuadSubChannelNormalSliceValues
    variable = mdot
    execute_on = final
    file_base = "mdot_In.txt"
    height = 0.0
  []
[]

[Executioner]
  type = Steady
[]
