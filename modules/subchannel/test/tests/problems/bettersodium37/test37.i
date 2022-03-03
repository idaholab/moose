T_in = 660
# [1e+6 kg/m^2-hour] turns into kg/m^2-sec
mass_flux_in = ${fparse 1e+6 * 300.00 / 36000.*0.5}
P_out = 2.0e5 # Pa
[Mesh]
  type = BetterTriSubChannelMesh
  nrings = 4
  n_cells = 100
  n_blocks = 1
  flat_to_flat = 0.077
  heated_length = 1.0
  rod_diameter = 0.01
  pitch = 0.012
  dwire = 0.002
  hwire = 0.0833
  spacer_z = '0'
  spacer_k = '5.0'
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
  [mu]
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
  type = BetterLiquidMetalSubChannel1PhaseProblem
  fp = sodium
  beta = 0.1
  P_out = 2.0e5
  CT = 1.0
  compute_density = true
  compute_viscosity = true
  compute_power = true
[]

[ICs]
  [S_IC]
    type = BetterTriFlowAreaIC
    variable = S
  []

  [w_perim_IC]
    type = BetterTriWettedPerimIC
    variable = w_perim
  []

   [q_prime_IC]
    type = BetterTriPowerIC
    variable = q_prime
    power = 1.000e6 # W
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
[]

[Executioner]
  type = Steady
  nl_rel_tol = 0.9
  l_tol = 0.9
[]
