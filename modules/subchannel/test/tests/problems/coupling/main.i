T_in = 297.039 # K
P_out = 101325 # Pa
heated_length = 1.0

[QuadSubChannelMesh]
  [sub_channel]
    type = SCMQuadSubChannelMeshGenerator
    nx = 2
    ny = 2
    n_cells = 10
    pitch = 0.014605
    pin_diameter = 0.012065
    gap = 0.0015875
    heated_length = ${heated_length}
    spacer_z = '0.0'
    spacer_k = '0.0'
  []

  [fuel_pins]
    type = SCMQuadPinMeshGenerator
    input = sub_channel
    nx = 2
    ny = 2
    n_cells = 10
    pitch = 0.014605
    heated_length = ${heated_length}
  []
[]

[AuxVariables]
  [mdot]
    block = sub_channel
  []
  [SumWij]
    block = sub_channel
  []
  [P]
    block = sub_channel
  []
  [DP]
    block = sub_channel
  []
  [h]
    block = sub_channel
  []
  [T]
    block = sub_channel
  []
  [Tpin]
    block = fuel_pins
  []
  [Dpin]
    block = fuel_pins
  []
  [rho]
    block = sub_channel
  []
  [mu]
    block = sub_channel
  []
  [S]
    block = sub_channel
  []
  [w_perim]
    block = sub_channel
  []
  [q_prime]
    block = fuel_pins
  []
  [displacement]
    block = sub_channel
  []
[]

[FluidProperties]
  [water]
    type = Water97FluidProperties
  []
[]

[Functions]
  [axial_heat_rate]
    type = ParsedFunction
    expression = '(pi/2)*sin(pi*z/L)'
    symbol_names = 'L'
    symbol_values = '${heated_length}'
  []
[]

[SubChannel]
  type = QuadSubChannel1PhaseProblem
  n_blocks = 1
  fp = water
  beta = 0.006
  CT = 2.6
  compute_density = true
  compute_viscosity = true
  compute_power = true
  P_out = ${P_out}
[]

[ICs]
  [S_IC]
    type = SCMQuadFlowAreaIC
    variable = S
  []

  [w_perim_IC]
    type = SCMQuadWettedPerimIC
    variable = w_perim
  []

  [q_prime_IC]
    type = SCMQuadPowerIC
    variable = q_prime
    power = 1000 # W
    filename = "power_profile.txt"
    axial_heat_rate = axial_heat_rate
  []

  [T_ic]
    type = ConstantIC
    variable = T
    value = ${T_in}
  []

  [Dpin_ic]
    type = ConstantIC
    variable = Dpin
    value = 0.012065
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
    mass_flux = 131.43435930715006
    execute_on = 'timestep_begin'
  []
[]

[Outputs]
  exodus = true
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  fixed_point_max_its = 30
  fixed_point_min_its = 2
  fixed_point_rel_tol = 1e-6
[]

################################################################################
# A multiapp that transfers data to heat conduction simulations
################################################################################

[MultiApps] # I have as many multiapps as pins
  [sub]
    type = FullSolveMultiApp
    input_files = sub.i # seperate file for multiapps due to radial power profile
    execute_on = 'timestep_end'
    positions = '0   0   0 '
    bounding_box_padding = '0 0 0.01'
  []
  [viz]
    type = FullSolveMultiApp
    input_files = '3d.i'
    execute_on = 'timestep_end'
  []
[]

[Transfers]
  [Tpin] # send pin surface temperature to heat conduction,
    type = MultiAppGeneralFieldNearestLocationTransfer
    to_multi_app = sub
    variable = Pin_surface_temperature
    source_variable = Tpin
    execute_on = 'timestep_end'
  []

  [from_sub] # send heat flux from heat conduction to subchannel
    type = MultiAppGeneralFieldNearestLocationTransfer
    from_multi_app = sub
    variable = q_prime
    source_variable = q_prime
    from_boundaries = right
    execute_on = 'timestep_end'
  []

  [subchannel_transfer]
    type = SCMSolutionTransfer
    to_multi_app = viz
    variable = 'mdot SumWij P DP h T rho mu S'
    execute_on = 'timestep_end'
  []

  [pin_transfer]
    type = SCMPinSolutionTransfer
    to_multi_app = viz
    variable = 'Tpin q_prime Dpin'
    execute_on = 'timestep_end'
  []
[]
