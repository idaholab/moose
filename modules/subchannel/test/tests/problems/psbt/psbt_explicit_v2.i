T_in = 359.15
# [1e+6 kg/m^2-hour] turns into kg/m^2-sec
mass_flux_in = '${fparse 1e+6 * 17.00 / 3600.}'
P_out = 4.923e6 # Pa
pin_diameter = 0.00950

[QuadSubChannelMesh]
  [sub_channel]
    type = SCMQuadSubChannelMeshGenerator
    nx = 6
    ny = 6
    n_cells = 3
    pitch = 0.0126
    pin_diameter = ${pin_diameter}
    gap = 0.00095 # the half gap between sub-channel assemblies
    heated_length = 1.0
    spacer_z = '0.0'
    spacer_k = '0.0'
  []

  [fuel_pins]
    type = SCMQuadPinMeshGenerator
    input = sub_channel
    nx = 6
    ny = 6
    n_cells = 3
    pitch = 0.0126
    heated_length = 1.0
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
  default_friction_model = false
  constant_beta = false
[]

[ICs]
  [S_IC]
    type = SCMQuadFlowAreaIC
    variable = S
  []

  [w_perim_IC]
    type = SCMQuadWettedPerimIC
    variable = w_perim
  []

  [T_ic]
    type = ConstantIC
    variable = T
    value = ${T_in}
  []

  [Dpin_ic]
    type = ConstantIC
    variable = Dpin
    value = ${pin_diameter}
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
    type = SCMMassFlowRateAux
    variable = mdot
    boundary = inlet
    area = S
    mass_flux = ${mass_flux_in}
    execute_on = 'timestep_begin'
  []
  [q_prime_IC]
    type = SCMQuadPowerAux
    variable = q_prime
    power = 1.0e6 # W
    filename = "power_profile.txt" #type in name of file that describes radial power profile
    execute_on = 'initial timestep_begin'
  []
[]

[Outputs]
  exodus = true
  csv = true
  [Temp_Out_MATRIX]
    type = QuadSubChannelNormalSliceValues
    variable = T
    execute_on = final
    file_base = "Temp_Out_Explicit.txt"
    height = 1.0
  []
[]

[Postprocessors]
  [total_pressure_drop]
    type = SubChannelDelta
    variable = P
    execute_on = "timestep_end"
  []
  [T1]
    type = SubChannelPointValue
    variable = T
    index = 0
    execute_on = "timestep_end"
    height = 1
  []
  [T2]
    type = SubChannelPointValue
    variable = T
    index = 7
    execute_on = "timestep_end"
    height = 1
  []
  [T3]
    type = SubChannelPointValue
    variable = T
    index = 14
    execute_on = "timestep_end"
    height = 1
  []
  [T4]
    type = SubChannelPointValue
    variable = T
    index = 21
    execute_on = "timestep_end"
    height = 1
  []
  [T5]
    type = SubChannelPointValue
    variable = T
    index = 28
    execute_on = "timestep_end"
    height = 1
  []
  [T6]
    type = SubChannelPointValue
    variable = T
    index = 35
    execute_on = "timestep_end"
    height = 1
  []
[]

[Executioner]
  type = Steady
[]
