######## BC's #################
T_in = 359.15
# [1e+6 kg/m^2-hour] turns into kg/m^2-sec
mass_flux_in = '${fparse 1e+6 * 17.00 / 3600.}'
P_out = 4.923e6 # Pa
heated_length = 1.0

[GlobalParams]
  ######## Geometry #
  nx = 2
  ny = 2
  n_cells = 25
  pitch = 0.0126
  pin_diameter = 0.00950
  gap = 0.00095
  heated_length = ${heated_length}
  spacer_z = '0.0'
  spacer_k = '0.0'
  power = 100000.0 # W
[]

[QuadSubChannelMesh]
  [subchannel]
    type = QuadSubChannelMeshGenerator
  []

  [fuel_pins]
    type = QuadPinMeshGenerator
    input = subchannel
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
  [mu]
    block = subchannel
  []
  [S]
    block = subchannel
  []
  [w_perim]
    block = subchannel
  []
  [q_prime]
    block = fuel_pins
  []
  [Tpin]
    block = fuel_pins
  []
  [Dpin]
    block = fuel_pins
  []
[]

[FluidProperties]
  [water]
    type = Water97FluidProperties
  []
[]

[SubChannel]
  type = QuadSubChannel1PhaseProblem
  fp = water
  n_blocks = 1
  beta = 0.006
  CT = 2.6
  compute_density = true
  compute_viscosity = true
  compute_power = true
  P_out = ${P_out}
  verbose_subchannel = true
  deformation = true
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
    value = 0.00950
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
    mass_flux = ${mass_flux_in}
    execute_on = 'timestep_begin'
  []
[]

[Outputs]
  exodus = true
  [Temp_Out_MATRIX]
    type = QuadSubChannelNormalSliceValues
    variable = T
    execute_on = final
    file_base = "Temp_Out.txt"
    height = 1.0
  []
  [mdot_Out_MATRIX]
    type = QuadSubChannelNormalSliceValues
    variable = mdot
    execute_on = final
    file_base = "mdot_Out.txt"
    height = 1.0
  []
  [mdot_In_MATRIX]
    type = QuadSubChannelNormalSliceValues
    variable = mdot
    execute_on = final
    file_base = "mdot_In.txt"
    height = 0.0
  []
[]

[UserObjects]
  [Tpin_avg_uo]
    type = NearestPointLayeredAverage
    direction = z
    num_layers = 1000
    variable = Tpin
    block = fuel_pins
    points = '0 0 0'
    execute_on = timestep_end
  []
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  fixed_point_max_its = 8
  fixed_point_min_its = 6
  fixed_point_rel_tol = 1e-6
[]

################################################################################
[MultiApps]
  ################################################################################
  # Couple to BISON
  ################################################################################
  [sub]
    type = FullSolveMultiApp
    app_type = BisonApp
    input_files = one_pin_problem_sub.i
    execute_on = 'timestep_end'
    positions = '0   0   0 '
    output_in_position = true
    bounding_box_padding = '0 0 0.01'
  []

  ################################################################################
  # A multiapp that projects data to a detailed mesh
  ################################################################################
  [viz]
    type = FullSolveMultiApp
    input_files = '3d.i'
    execute_on = 'FINAL'
  []
[]

[Transfers]
  [Tpin] # send pin surface temperature to bison,
    type = MultiAppGeneralFieldNearestLocationTransfer
    to_multi_app = sub
    variable = Pin_surface_temperature
    source_variable = Tpin
    execute_on = 'timestep_end'
  []

  [diameter] # send diameter information from /BISON/heatConduction to subchannel
    type =  MultiAppGeneralFieldNearestLocationTransfer
    from_multi_app = sub
    variable = Dpin
    source_variable = pin_diameter_deformed
    from_boundaries = right
    execute_on = 'timestep_end'
  []

  [q_prime] # send heat flux from /BISON/heatConduction to subchannel
    type = MultiAppGeneralFieldNearestLocationTransfer
    from_multi_app = sub
    variable = q_prime
    source_variable = q_prime
    from_boundaries = right
    execute_on = 'timestep_end'
  []

  [subchannel_transfer]
    type = MultiAppGeneralFieldNearestLocationTransfer
    to_multi_app = viz
    variable = 'mdot SumWij P DP h T rho mu S w_perim'
  []

  [pin_transfer]
    type = MultiAppDetailedPinSolutionTransfer
    to_multi_app = viz
    variable = 'Tpin Dpin q_prime'
  []
[]
