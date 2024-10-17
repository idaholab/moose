[GlobalParams]
  ######## Geometry #
  nx = 5
  ny = 5
  n_cells = 36
  n_blocks = 1
  pitch = 0.0126
  pin_diameter = 0.00950
  gap = 0.00095
  heated_length = 3.6
  spacer_z = '0.0'
  spacer_k = '0.0'
[]

######## BC's #################
T_in = 560.15
mass_flux_in = 3035.27 # kg/m^2-sec
P_out = 15.5e6 # Pa
power = 1.0656e6 # W
heated_length = 3.6 # m

##### Geometry #####
pitch = 0.0126

[QuadSubChannelMesh]
  [sub_channel]
    type = QuadSubChannelMeshGenerator
  []

  [fuel_pins]
    type = QuadPinMeshGenerator
    input = sub_channel
  []
[]

[Functions]
  [axial_heat_rate]
    type = ParsedFunction
    value = '(pi/2)*sin(pi*z/L)'
    vars = 'L'
    vals = '${heated_length}'
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
[]

[FluidProperties]
  [water]
    type = Water97FluidProperties
  []
[]

[SubChannel]
  type = QuadSubChannel1PhaseProblem
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
    type = QuadFlowAreaIC
    variable = S
  []

  [w_perim_IC]
    type = QuadWettedPerimIC
    variable = w_perim
  []

  [q_prime_IC]
    type = QuadPowerIC
    variable = q_prime
    power = ${power} # W
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
    value = ${pin_diameter}
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
    type = MassFlowRateAux
    variable = mdot
    boundary = inlet
    area = S
    mass_flux = ${mass_flux_in}
    execute_on = 'timestep_begin'
  []
[]

[UserObjects]
  [Tpin_avg_uo]
    type = NearestPointLayeredAverage
    direction = z
    num_layers = 1000
    variable = Tpin
    block = fuel_pins
    points = '${fparse -1.5 * pitch}   ${fparse -1.5 * pitch}   0
              ${fparse -0.5 * pitch}   ${fparse -1.5 * pitch}   0
              ${fparse  0.5 * pitch}   ${fparse -1.5 * pitch}   0
              ${fparse  1.5 * pitch}   ${fparse -1.5 * pitch}   0

              ${fparse -1.5 * pitch}   ${fparse -0.5 * pitch}   0
              ${fparse -0.5 * pitch}   ${fparse -0.5 * pitch}   0
              ${fparse  0.5 * pitch}   ${fparse -0.5 * pitch}   0
              ${fparse  1.5 * pitch}   ${fparse -0.5 * pitch}   0

              ${fparse -1.5 * pitch}   ${fparse  0.5 * pitch}   0
              ${fparse -0.5 * pitch}   ${fparse  0.5 * pitch}   0
              ${fparse  0.5 * pitch}   ${fparse  0.5 * pitch}   0
              ${fparse  1.5 * pitch}   ${fparse  0.5 * pitch}   0

              ${fparse -1.5 * pitch}   ${fparse  1.5 * pitch}   0
              ${fparse -0.5 * pitch}   ${fparse  1.5 * pitch}   0
              ${fparse  0.5 * pitch}   ${fparse  1.5 * pitch}   0
              ${fparse  1.5 * pitch}   ${fparse  1.5 * pitch}   0'
    execute_on = timestep_end
  []
[]

[Outputs]
  exodus = true
  [Temp_Out_MATRIX]
    type = QuadSubChannelNormalSliceValues
    variable = T
    execute_on = final
    file_base = "Temp_Out.txt"
    height = 3.6
  []
  [mdot_Out_MATRIX]
    type = QuadSubChannelNormalSliceValues
    variable = mdot
    execute_on = final
    file_base = "mdot_Out.txt"
    height = 3.6
  []
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  fixed_point_max_its = 2
  fixed_point_min_its = 1
  fixed_point_rel_tol = 1e-6
[]

################################################################################
# A multiapp that transfers data to BISON/heatconduction simulations
################################################################################

[MultiApps] # I can have as many multiapps as pins
  [sub]
    app_type = BisonApp
    type = FullSolveMultiApp
    input_files = 'sub_hot_corner.i sub_cold_corner.i'
    execute_on = 'timestep_end'
    positions = '
        ${fparse -1.5 * pitch}   ${fparse -1.5 * pitch}   0
        ${fparse  1.5 * pitch}   ${fparse  1.5 * pitch}   0'
    output_in_position = true
    bounding_box_padding = '0 0 0.1'
  []

  # [sub1]
  #   app_type = BisonApp
  #   type = FullSolveMultiApp
  #   input_files = sub_hot_corner.i # seperate file for multiapps due to radial power profile (each sub has different power)
  #   execute_on = 'timestep_end'
  #   positions = '${fparse -1.5 * pitch}   ${fparse -1.5 * pitch}   0'
  #   output_in_position = true
  #   bounding_box_padding = '0 0 0.1'
  # []
  #
  # [sub2]
  #   app_type = BisonApp
  #   type = FullSolveMultiApp
  #   input_files = sub_cold_corner.i # seperate file for multiapps due to radial power profile (each sub has different power)
  #   execute_on = 'timestep_end'
  #   positions = '${fparse 1.5 * pitch}   ${fparse 1.5 * pitch}   0'
  #   output_in_position = true
  #   bounding_box_padding = '0 0 0.1'
  # []

  [viz]
    type = FullSolveMultiApp
    input_files = "detailedMesh.i"
    execute_on = "final"
  []
[]

[Transfers]
  [Tpin] # send pin surface temperature to bison,
    type = MultiAppUserObjectTransfer2
    to_multi_app = sub
    variable = Pin_surface_temperature
    user_object = Tpin_avg_uo
  []

  [q_prime]
    type = MultiAppUserObjectTransfer2
    from_multi_app = sub
    variable = q_prime
    user_object = q_prime_uo
    execute_on = 'timestep_end'
  []

  # [Tpin1] # send pin surface temperature to bison,
  #   type = MultiAppUserObjectTransfer2
  #   to_multi_app = sub1
  #   variable = Pin_surface_temperature
  #   user_object = Tpin_avg_uo
  # []
  #
  # [Tpin2] # send pin surface temperature to bison,
  #   type = MultiAppUserObjectTransfer2
  #   to_multi_app = sub2
  #   variable = Pin_surface_temperature
  #   user_object = Tpin_avg_uo
  # []
  #
  # [q_prime1] # send heat flux from BISON/heatConduction to subchannel
  #   type = MultiAppUserObjectTransfer2
  #   from_multi_app = sub1
  #   variable = q_prime
  #   user_object = q_prime_uo
  #   execute_on = 'timestep_end'
  # []
  #
  # [q_prime2] # send heat flux from BISON/heatConduction to subchannel
  #   type = MultiAppUserObjectTransfer2
  #   from_multi_app = sub2
  #   variable = q_prime
  #   user_object = q_prime_uo
  #   execute_on = 'timestep_end'
  # []

  ###### Transfers to the detailedMesh at the end of the coupled simulations
  [subchannel_transfer]
    type = MultiAppDetailedSolutionTransfer
    to_multi_app = viz
    variable = 'mdot SumWij P DP h T rho mu S'
  []
  [pin_transfer]
    type = MultiAppDetailedPinSolutionTransfer
    to_multi_app = viz
    variable = 'Tpin q_prime'
  []
[]
