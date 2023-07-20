# J. T. Han 1977,
# This input file models a block next to the duct of  the of the assembly
# 102 mm above the start of the heated section.
# T_in = 596.75 # K, high flow case
T_in = 541.55 #K, low flow case
A12 = 1.00423e3
A13 = -0.21390
A14 = -1.1046e-5
rho = ${fparse A12 + A13 * T_in + A14 * T_in * T_in}
# inlet_vel = 6.93 #m/sec, high flow case
inlet_vel = 0.48 #m/sec, low flow case
mass_flux_in = ${fparse rho *  inlet_vel}
P_out = 2.0e5 # Pa
[TriSubChannelMesh]
  [subchannel]
    type = TriSubChannelMeshGenerator
    nrings = 3
    n_cells = 50
    flat_to_flat = 0.0324290
    heated_length = 0.4572
    unheated_length_entry = 0.4064
    unheated_length_exit = 0.1524
    rod_diameter = 0.005842
    pitch = 7.2644e-3
    dwire = 0.0014224
    hwire = 0.3048
    spacer_z = '0.0'
    spacer_k = '0.0'
    z_blockage = '0.49 0.52'
    index_blockage = '29 31 30 32 34 33 35 15 16 8 17 18 9 19'
    reduction_blockage = '0.08 0.08 0.08 0.08 0.08 0.08 0.08 0.08 0.08 0.08 0.08 0.08 0.08 0.08'
    k_blockage = '6 6 6 6 6 6 6 6 6 6 6 6 6 6 '
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
  [w_perim]
  []
  [q_prime]
  []
  [mu]
  []
[]

[FluidProperties]
  [sodium]
      type = PBSodiumFluidProperties
  []
[]

[Problem]
  type = LiquidMetalSubChannel1PhaseProblem
  fp = sodium
  n_blocks = 1
  P_out = 2.0e5
  CT = 10
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
    # power = 145000  #W, high flow case
    power = 52800  #W, low flow case
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
  csv = true
[]

[Postprocessors]
  [1]
    type = SubChannelPointValue
    variable = T
    index = 34
    execute_on = 'initial timestep_end'
    height = 0.94
  []
  [2]
    type = SubChannelPointValue
    variable = T
    index = 33
    execute_on = 'initial timestep_end'
    height = 0.94
  []
  [3]
    type = SubChannelPointValue
    variable = T
    index = 18
    execute_on = 'initial timestep_end'
    height = 0.94
  []
  [4]
    type = SubChannelPointValue
    variable = T
    index = 9
    execute_on = 'initial timestep_end'
    height = 0.94
  []
  [5]
    type = SubChannelPointValue
    variable = T
    index = 3
    execute_on = 'initial timestep_end'
    height = 0.94
  []
  [6]
    type = SubChannelPointValue
    variable = T
    index = 0
    execute_on = 'initial timestep_end'
    height = 0.94
  []
  [7]
    type = SubChannelPointValue
    variable = T
    index = 12
    execute_on = 'initial timestep_end'
    height = 0.94
  []
  [8]
    type = SubChannelPointValue
    variable = T
    index = 25
    execute_on = 'initial timestep_end'
    height = 0.94
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
    input_files = "FFM-5Bdetailed.i"
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
