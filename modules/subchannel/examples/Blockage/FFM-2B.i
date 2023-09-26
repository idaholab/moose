# M. H. Fontana et al 1973, 1976, Inlet blockage, Case 719.
# This input file models a block at the inlet of the assembly,
# using the aux kernel BlockedMassFlowRateAux. The affected subchannels get a mass flux BC that is
# user defined to be very low.
T_in = 589.15
A12 = 1.00423e3
A13 = -0.21390
A14 = -1.1046e-5
rho = ${fparse A12 + A13 * T_in + A14 * T_in * T_in}
Total_surface_area = 0.000452826 #m2
Blocked_surface_area = ${fparse 8.65158e-06 * 13}
Flow_area = ${fparse Total_surface_area - Blocked_surface_area}
vol_flow = 0.00341 #m3/sec
mass_flux_in = ${fparse rho *  vol_flow / Flow_area}
P_out = 2.0e5 # Pa
[TriSubChannelMesh]
  [subchannel]
    type = TriSubChannelMeshGenerator
    nrings = 3
    n_cells = 50
    flat_to_flat = 0.0338514
    heated_length = 0.5334
    unheated_length_entry = 0.0762
    unheated_length_exit = 0.3048
    rod_diameter = 0.005842
    pitch = 7.2644e-3
    dwire = 0.0014224
    hwire = 0.3048
    spacer_z = '0.0'
    spacer_k = '0.0'
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
  P_out = 2.0e5
  CT = 1.0
  compute_density = true
  compute_viscosity = true
  compute_power = true
  P_tol = 1.0e-4
  T_tol = 1.0e-4
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
    power = 162153.6 #W
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
    type = BlockedMassFlowRateAux
    variable = mdot
    boundary = inlet
    index_blockage = '0 1 2 3 4 5 11 22 21 10 20 19 9'
    area = S
    unblocked_mass_flux = ${mass_flux_in}
    blocked_mass_flux = ${fparse 0.1 * mass_flux_in}
    execute_on = 'timestep_begin'
  []
[]

[Outputs]
  exodus = true
  csv = true
[]

[Postprocessors]
  [1]
    type = SubChannelPointValue
    variable = T
    index = 0
    execute_on = 'initial timestep_end'
    height = 0.152
  []
  [2]
    type = SubChannelPointValue
    variable = T
    index = 5
    execute_on = 'initial timestep_end'
    height = 0.152
  []
  [3]
    type = SubChannelPointValue
    variable = T
    index = 3
    execute_on = 'initial timestep_end'
    height = 0.152
  []
  [4]
    type = SubChannelPointValue
    variable = T
    index = 1
    execute_on = 'initial timestep_end'
    height = 0.152
  []
  [5]
    type = SubChannelPointValue
    variable = T
    index = 6
    execute_on = 'initial timestep_end'
    height = 0.152
  []
  [6]
    type = SubChannelPointValue
    variable = T
    index = 36
    execute_on = 'initial timestep_end'
    height = 0.152
  []
[]

[Executioner]
  type = Steady
[]

################################################################################
# A multiapp that projects data to a detailed mesh
################################################################################

[MultiApps]
  [viz]
    type = FullSolveMultiApp
    input_files = "FFM-2Bdetailed.i"
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
