################################################################################
## THORS bundle 5B partial edge blockage benchmark                            ##
## SCM simulation, high flow case                                             ##
## POC : Vasileios Kyriakopoulos, vasileios.kyriakopoulos@inl.gov             ##
################################################################################
# Details on the experimental facility modeled can be found at:
# Han, J. T. "Blockages in LMFBR fuel assemblies: A review of experimental and theoretical studies." (1977).
# This input file models a block next to the duct of  the of the assembly
# 102 mm above the start of the heated section.

# Boundary conditions
T_in = 596.75 # K, high flow case
A12 = 1.00423e3
A13 = -0.21390
A14 = -1.1046e-5
rho = '${fparse A12 + A13 * T_in + A14 * T_in * T_in}'
inlet_vel = 6.93 #m/sec, high flow case
mass_flux_in = '${fparse rho *  inlet_vel}'
P_out = 2.0e5 # Pa
[TriSubChannelMesh]
  [subchannel]
    type = SCMTriSubChannelMeshGenerator
    nrings = 3
    n_cells = 50
    flat_to_flat = 0.0324290
    heated_length = 0.4572
    unheated_length_entry = 0.4064
    unheated_length_exit = 0.1524
    pin_diameter = 0.005842
    pitch = 7.2644e-3
    dwire = 0.0014224
    hwire = 0.3048
    z_blockage = '0.49 0.52'
    index_blockage = '29 31 30 32 34 33 35 15 16 8 17 18 9 19'
    reduction_blockage = '0.1 0.1 0.1 0.1 0.1 0.1 0.1 0.1 0.1 0.1 0.1 0.1 0.1 0.1'
    k_blockage = '5 5 5 5 5 5 5 5 5 5 5 5 5 5 '
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
  CT = 2
  compute_density = true
  compute_viscosity = true
  compute_power = true
  P_tol = 1.0e-4
  T_tol = 1.0e-4
  implicit = true
  segregated = false
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
    power = 145000  #W, high flow case
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
    input_files = "FFM-5B_viz.i"
    execute_on = "timestep_end"
  []
[]

[Transfers]
  [xfer]
    type = SCMSolutionTransfer
    to_multi_app = viz
    variable = 'mdot SumWij P DP h T rho mu q_prime S displacement w_perim'
  []
[]
