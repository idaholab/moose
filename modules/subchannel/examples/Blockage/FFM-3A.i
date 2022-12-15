# M. H. Fontana et al 1973, 1976
# This input file models a partial block at the center of the assembly
# The affected subchannels get an area reduction and a form loss coefficient
T_in = 714.261
A12 = 1.00423e3
A13 = -0.21390
A14 = -1.1046e-5
rho = ${fparse A12 + A13 * T_in + A14 * T_in * T_in}
Total_surface_area = 0.000467906 #m2
Blocked_surface_area = 0.0 #m2
Flow_area = ${fparse Total_surface_area - Blocked_surface_area}
vol_flow = 3.41E-03 #m3/s
mass_flux_in = ${fparse rho *  vol_flow / Flow_area}
P_out = 2.0e5 # Pa
[TriSubChannelMesh]
  [subchannel]
    type = TriSubChannelMeshGenerator
    nrings = 3
    n_cells = 36
    flat_to_flat = 3.41e-2
    heated_length = 0.5334
    unheated_length_entry = 0.3048
    unheated_length_exit = 0.0762
    rod_diameter = 5.84e-3
    pitch = 7.26e-3
    dwire = 1.42e-3
    hwire = 0.3048
    spacer_z = '0.0'
    spacer_k = '0.0'
    z_blockage = '0.6858 0.69215'
    index_blockage = '0 1 2 3 4 5'
    reduction_blockage = '0.1 0.1 0.1 0.1 0.1 0.1'
    k_blockage = '0.0 0.0 0.0 0.0 0.0 0.0'
    # z_blockage = '0.6 0.80'
    # index_blockage = '0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41'
    # reduction_blockage = '.1 .1 .1 .1 .1 .1 .1 .1 .1 .1
    #                       .1 .1 .1 .1 .1 .1 .1 .1 .1 .1
    #                       .1 .1 .1 .1 .1 .1 .1 .1 .1 .1
    #                       .1 .1 .1 .1 .1 .1 .1 .1 .1 .1
    #                                               .1 .1'
    # k_blockage = '.0 .0 .0 .0 .0 .0 .0 .0 .0 .0
    #               .0 .0 .0 .0 .0 .0 .0 .0 .0 .0
    #               .0 .0 .0 .0 .0 .0 .0 .0 .0 .0
    #               .0 .0 .0 .0 .0 .0 .0 .0 .0 .0
    #                                       .0 .0'
  []
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
  type = LiquidMetalSubChannel1PhaseProblem
  fp = sodium
  n_blocks = 1
  beta = 0.006
  P_out = 2.0e5
  CT = 2.6
  compute_density = true
  compute_viscosity = true
  compute_power = true
  P_tol = 1.0e-5
  T_tol = 1.0e-5
  implicit = true
  segregated = false
  interpolation_scheme = 'upwind'
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
    power = 332500.0 #W
    filename = "pin_power_profile_19.txt"
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

[Postprocessors]
  [T37]
    type = TriSubChannelPointValue
    variable = T
    index = 37
    execute_on ='TIMESTEP_END'
    height = 0.9144
  []
  [T36]
    type = TriSubChannelPointValue
    variable = T
    index = 36
    execute_on ='TIMESTEP_END'
    height = 0.9144
  []
  [T20]
    type = TriSubChannelPointValue
    variable = T
    index = 20
    execute_on ='TIMESTEP_END'
    height = 0.9144
  []
  [T10]
    type = TriSubChannelPointValue
    variable = T
    index = 10
    execute_on ='TIMESTEP_END'
    height = 0.9144
  []
  [T4]
    type = TriSubChannelPointValue
    variable = T
    index = 4
    execute_on ='TIMESTEP_END'
    height = 0.9144
  []
  [T1]
    type = TriSubChannelPointValue
    variable = T
    index = 1
    execute_on ='TIMESTEP_END'
    height = 0.9144
  []
  [T14]
    type = TriSubChannelPointValue
    variable = T
    index = 14
    execute_on ='TIMESTEP_END'
    height = 0.9144
  []
  [T28]
    type = TriSubChannelPointValue
    variable = T
    index = 28
    execute_on ='TIMESTEP_END'
    height = 0.9144
  []
[]

################################################################################
# A multiapp that projects data to a detailed mesh
################################################################################

[MultiApps]
  [viz]
    type = FullSolveMultiApp
    input_files = "FFM-3Adetailed.i"
    execute_on = "timestep_end"
  []
[]

[Transfers]
  [xfer]
    type = MultiAppDetailedSolutionTransfer
    to_multi_app = viz
    variable = 'mdot SumWij P DP h T rho mu q_prime S'
  []
[]
