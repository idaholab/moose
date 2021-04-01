T_in = 359.15
length = 3.658
# [1e+6 kg/m^2-hour] turns into kg/m^2-sec
# mass_flux_in = ${fparse 1e+6 * 17.00 / 3600.}
P_out = 4.923e6 # Pa
mass_flow_in = 11.522 #kg/sec

[Mesh]
  type = QuadSubChannelMesh
  nx = 6
  ny = 6
  max_dz = 0.02
  pitch = 0.0126
  rod_diameter = 0.00950
  gap = 0.00095
  heated_length = ${length}
  spacer_z = '0 0.229 0.457 0.686 0.914 1.143 1.372 1.600 1.829 2.057 2.286 2.515 2.743 2.972 3.200 3.429'
  spacer_k = '0.7 0.4 1.0 0.4 1.0 0.4 1.0 0.4 1.0 0.4 1.0 0.4 1.0 0.4 1.0 0.4'
[]

[AuxVariables]
  [mdot]
  []
  [SumWij]
  []
  [P]
  []
  [DP]
  []
  [h]
  []
  [T]
  []
  [rho]
  []
  [S]
  []
  [Sij]
  []
  [w_perim]
  []
  [q_prime]
  []
[]

[Functions]
  [axial_heat_rate]
    type = ParsedFunction
    value = '(pi/2)*sin(pi*z/L)'
    vars = 'L'
    vals = '${length}'
  []
[]

[Modules]
  [FluidProperties]
    [water]
      type = Water97FluidProperties
    []
  []
[]

[Problem]
  type = LiquidWaterSubChannel1PhaseProblem
  fp = water
  abeta = 0.08
  CT = 1.0
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
    power = 3.44e6 # W
    filename = "power_profile.txt" #type in name of file that describes power profile
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
    value = ${P_out}
  []

  [DP_ic]
    type = ConstantIC
    variable = DP
    value = 0.0
  []

  [rho_ic]
    type = RhoFromPressureTemperatureIC
    variable = rho
    p = P
    T = T
    fp = water
  []

  [h_ic]
    type = SpecificEnthalpyFromPressureTemperatureIC
    variable = h
    p = P
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
    type = ConstantAux
    variable = P
    boundary = outlet
    value = ${P_out}
    execute_on = 'timestep_begin'
  []
  [T_in_bc]
    type = ConstantAux
    variable = T
    boundary = inlet
    value = ${T_in}
    execute_on = 'timestep_begin'
  []
    [mdot_in_bc]
      type = UniformlyDistributedMassFlowRateAux
      variable = mdot
      boundary = inlet
      mass_flow = ${mass_flow_in}
      execute_on = 'timestep_begin'
    []
[]

[Outputs]
  exodus = true
  [Temp_Out_MATRIX]
    type = NormalSliceValues
    variable = T
    execute_on = final
    file_base = "Temp_Out.txt"
    height = 3.658
  []
  [mdot_Out_MATRIX]
    type = NormalSliceValues
    variable = mdot
    execute_on = final
    file_base = "mdot_Out.txt"
    height = 3.658
  []
  [mdot_In_MATRIX]
    type = NormalSliceValues
    variable = mdot
    execute_on = final
    file_base = "mdot_In.txt"
    height = 0.0
  []
[]

[Executioner]
  type = Steady
  nl_rel_tol = 0.9
  l_tol = 0.9
[]
