T_in = 660
mass_flux_in = '${fparse 1e+6 * 37.00 / 36000.*0.5}'
P_out = 2.0e5 # Pa
[TriSubChannelMesh]
  [subchannel]
    type = SCMTriSubChannelMeshGenerator
    nrings = 4
    n_cells = 100
    flat_to_flat = 0.085
    heated_length = 1.0
    pin_diameter = 0.01
    pitch = 0.012
    dwire = 0.002
    hwire = 0.0833
    spacer_z = '0 0.2 0.4 0.6 0.8'
    spacer_k = '0.1 0.1 0.1 0.1 0.10'
  []

  [duct]
    type = SCMTriDuctMeshGenerator
    input = subchannel
    nrings = 4
    n_cells = 100
    flat_to_flat = 0.085
    heated_length = 1.0
    pitch = 0.012
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
  [q_prime]
    block = subchannel
  []
  [mu]
    block = subchannel
  []
  [displacement]
    block = subchannel
  []
  [duct_heat_flux]
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
  compute_density = false
  compute_viscosity = false
  compute_power = true
  P_tol = 1.0e-5
  T_tol = 1.0e-5
  implicit = true
  segregated = false
  staggered_pressure = false
  verbose_multiapps = true
  verbose_subchannel = false
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
    power = 1e5 #1.000e5 # W
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

  [T_duct_ic]
    type = ConstantIC
    variable = Tduct
    value = ${T_in}
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

[UserObjects]
  [Tduct_avg_uo]
    type = NearestPointLayeredAverage
    direction = z
    num_layers = 1000
    variable = Tduct
    block = duct
    points = '0 0 0'
    execute_on = 'TIMESTEP_END'
  []
[]

[Outputs]
  exodus = true
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  fixed_point_max_its = 2
  fixed_point_min_its = 2
  fixed_point_rel_tol = 1e-6
[]

################################################################################
# A multiapp that projects data to a detailed mesh
################################################################################

[MultiApps]
  # Multiapp to duct heat conduction module
  [duct_map]
    type = FullSolveMultiApp
    input_files = wrapper.i # seperate file for multiapps due to radial power profile
    execute_on = 'timestep_end'
    positions = '0   0   0' #center of assembly
    bounding_box_padding = '10.0 10.0 10.0'
  []

  # Multiapp to detailed mesh for vizualization
  [viz]
    type = FullSolveMultiApp
    input_files = "3d.i"
    execute_on = 'timestep_end'
  []
[]

[Transfers]
  [duct_temperature_transfer] # Send duct temperature to heat conduction
    type = MultiAppInterpolationTransfer
    to_multi_app = duct_map
    source_variable = Tduct
    variable = duct_surface_temperature
  []

  [displacement_transfer]
    type = MultiAppGeneralFieldNearestNodeTransfer
    from_multi_app = duct_map
    source_variable = disp_magnitude
    variable = displacement
  []

  [q_prime] # Recover q_prime from heat conduction solve
    type = MultiAppInterpolationTransfer
    from_multi_app = duct_map
    source_variable = q_prime
    variable = duct_heat_flux
  []

  [xfer]
    type = SCMSolutionTransfer
    to_multi_app = viz
    variable = 'mdot SumWij P DP h T rho mu q_prime S displacement'
  []
[]
