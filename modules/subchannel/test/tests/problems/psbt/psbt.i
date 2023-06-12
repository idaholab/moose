T_in = 359.15
# [1e+6 kg/m^2-hour] turns into kg/m^2-sec
mass_flux_in = ${fparse 1e+6 * 17.00 / 3600.}
P_out = 4.923e6 # Pa

[QuadSubChannelMesh]
  [sub_channel]
    type = QuadSubChannelMeshGenerator
    nx = 6
    ny = 6
    n_cells = 3
    pitch = 0.0126
    rod_diameter = 0.00950
    gap = 0.00095 # the half gap between sub-channel assemblies
    heated_length = 1.0
    spacer_z = '0.0'
    spacer_k = '0.0'
  []

  [fuel_pins]
    type = QuadPinMeshGenerator
    input = sub_channel
    nx = 6
    ny = 6
    n_cells = 3
    pitch = 0.0126
    heated_length = 1.0
  []
[]

[Modules]
  [FluidProperties]
    [water]
      type = Water97FluidProperties
    []
  []
[]

[SubChannel]
  type = LiquidWaterSubChannel1PhaseProblem
  fp = water
  n_blocks = 1
  beta = 0.006
  CT = 2.6
  compute_density = true
  compute_viscosity = true
  compute_power = true
  P_out = ${P_out}
  verbose_subchannel = true
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
    power = 1.0e6 # W
    filename = "power_profile.txt" #type in name of file that describes radial power profile
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
  [P_out_bc]
    type = PostprocessorConstantAux
    variable = P
    boundary = outlet
    postprocessor = report_pressure_outlet
    execute_on = 'timestep_begin'
    block = sub_channel
  []
  [T_in_bc]
    type = ConstantAux
    variable = T
    boundary = inlet
    value = ${T_in}
    execute_on = 'timestep_begin'
    block = sub_channel
  []
  [mdot_in_bc]
    type = PostprocessorMassFlowRateAux
    variable = mdot
    boundary = inlet
    area = S
    postprocessor = report_mass_flux_inlet
    execute_on = 'timestep_begin'
    block = sub_channel
  []
[]

[Postprocessors]
  [report_mass_flux_inlet]
    type = Receiver
    default = ${mass_flux_in}
  []

  [report_pressure_outlet]
    type = Receiver
    default = ${P_out}
  []
[]

[Outputs]
  exodus = true
  [Temp_Out_MATRIX]
    type = QuadSubChannelNormalSliceValues
    variable = T
    execute_on = final
    file_base = "Temp_Out_Explicit.txt"
    height = 1.0
  []
[]

[Executioner]
  type = Steady
  nl_rel_tol = 0.9
  l_tol = 0.9
[]
