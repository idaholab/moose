T_in = 660
# [1e+6 kg/m^2-hour] turns into kg/m^2-sec
mass_flux_in = ${fparse 1e+6 * 17.00 / 36000.*0.5}
P_out = 2.0e5 # Pa
#mass_flow_in = 13.898 #kg/sec
# total axial coolant flow area is computed as 0.0029431 m2
[Mesh]
  type = TriSubChannelMesh
  nrings = 4
  flat_to_flat = 0.077
  heated_length = 3.658
  rod_diameter = 0.01
  pitch = 0.012
  dwire = 0.002
  hwire = 0.0833
  max_dz = 0.02
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

[Modules]
  [FluidProperties]
    [sodium]
       type = PBSodiumFluidProperties
    []
  []
[]

[Problem]
  type = LiquidMetalSubChannel1PhaseProblem
  fp = sodium
  abeta = 0.01
  CT = 1.0
  enforce_uniform_pressure = false
[]

[ICs]
  [S_IC]
    type = TriFlowAreaIC
    variable = S
  []

  [w_perim_IC]
    type = TriWettedPerimIC
    variable = w_perim
  []

   [q_prime_IC]
    type = TriPowerIC
    variable = q_prime
    power = 1.500e5 # W
    filename = "pin_power_profile37.txt"
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
    fp = sodium
  []

  [h_ic]
    type = SpecificEnthalpyFromPressureTemperatureIC
    variable = h
    p = P
    T = T
    fp = sodium
  []

#  [mdot_ic]
#    type = MassFlowRateIC
#    variable = mdot
#    area = S
#    mass_flux = ${mass_flux_in}
#  []

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
 # [Temp_Out_MATRIX]
 #   type = NormalSliceValues
 #   variable = T
 #   execute_on = final
 #   file_base = "Temp_Out.txt"
 #   height = 3.658
 # []
 # [mdot_Out_MATRIX]
 #   type = NormalSliceValues
 #   variable = mdot
 #   execute_on = final
 #   file_base = "mdot_Out.txt"
 #   height = 3.658
 # []
 # [mdot_In_MATRIX]
 #   type = NormalSliceValues
 #   variable = mdot
 #   execute_on = final
 #   file_base = "mdot_In.txt"
 #   height = 0.0
 # []
[]

[Executioner]
  type = Steady
  nl_rel_tol = 0.9
  l_tol = 0.9
[]
