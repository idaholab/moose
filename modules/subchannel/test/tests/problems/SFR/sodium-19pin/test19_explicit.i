T_in = 660
mass_flux_in = '${fparse 1e+6 * 300.00 / 36000.*0.5}'
P_out = 2.0e5 # Pa

[GlobalParams]
  nrings = 3
  n_cells = 5
  flat_to_flat = 0.056
  heated_length = 0.5
  pitch = 0.012
[]

[TriSubChannelMesh]
  [subchannel]
    type = SCMTriSubChannelMeshGenerator
    pin_diameter = 0.01
    dwire = 0.002
    hwire = 0.0833
    spacer_z = '0'
    spacer_k = '5.0'
  []
  [duct]
    type = SCMTriDuctMeshGenerator
    input = subchannel
  []
[]

[AuxVariables]
  [mdot]
    block = subchannel
  []
  [SumWij]
    block = subchannel
  []
  [P]
    block = subchannel
  []
  [DP]
    block = subchannel
  []
  [h]
    block = subchannel
  []
  [T]
    block = subchannel
  []
  [rho]
    block = subchannel
  []
  [S]
    block = subchannel
  []
  [w_perim]
    block = subchannel
  []
  [mu]
    block = subchannel
  []
  [q_prime]
    block = subchannel
  []
  [displacement]
    block = subchannel
  []
  [q_prime_duct]
    block = duct
  []
  [Tduct]
    block = duct
  []
[]

[FluidProperties]
  [sodium]
    type = PBSodiumFluidProperties
  []
[]

[Problem]
  type = TriSubChannel1PhaseProblem
  fp = sodium
  n_blocks = 1
  P_out = 2.0e5
  CT = 1.0
  compute_density = true
  compute_viscosity = true
  compute_power = true
  implicit = false
  segregated = true
  verbose_subchannel = true
[]

[ICs]
  [S_IC]
    type = SCMTriFlowAreaIC
    variable = S
  []

  [w_perim_IC]
    type = SCMTriWettedPerimIC
    variable = w_perim
  []

  [q_prime_IC]
    type = SCMTriPowerIC
    variable = q_prime
    power = 1000.0 # W
    filename = "pin_power_profile19.txt"
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
    fp = sodium
  []

  [rho_ic]
    type = RhoFromPressureTemperatureIC
    variable = rho
    p = ${P_out}
    T = T
    fp = sodium
  []

  [h_ic]
    type = SpecificEnthalpyFromPressureTemperatureIC
    variable = h
    p = ${P_out}
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

[Outputs]
  exodus = true
  csv = true
[]

[Postprocessors]
  [total_pressure_drop]
    type = SubChannelDelta
    variable = P
    execute_on = "timestep_end"
  []
[]

[Executioner]
  type = Steady
[]
