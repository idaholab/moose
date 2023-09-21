# M. Fontana, et All, VARIATION of subassembly made to have only one pin
# “Temperature distribution in the duct wall and at the exit of a 19-rod simulated lmfbr fuel assembly (ffm bundle 2a),
# ”Nuclear Technology, vol. 24, no. 2, pp. 176–200, 1974.
T_in = 588.5
mass_flux_in = ${fparse 1e+6 * 17.00 / 3600.}
P_out = 2.0e5 # Pa
rod_diameter = 5.84e-3
heated_length = 1.0
pitch = 7.26e-3
[TriSubChannelMesh]
  [subchannel]
    type = TriSubChannelMeshGenerator
    nrings = 2
    n_cells = 40
    flat_to_flat = 2.1e-2 # 3.41e-2
    heated_length = ${heated_length} # 0.5334
    rod_diameter = ${rod_diameter}
    pitch = ${pitch}
    dwire = 1.42e-3
    hwire = 0.3048
    spacer_z = '0'
    spacer_k = '0'
  []

  [fuel_pins]
    type = TriPinMeshGenerator
    input = subchannel
    nrings = 2
    n_cells = 40
    heated_length = ${heated_length} # 0.5334
    pitch = ${pitch}
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
  [S]
    block = subchannel
  []
  [Sij]
    block = subchannel
  []
  [w_perim]
    block = subchannel
  []
  [mu]
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
  [sodium]
      type = PBSodiumFluidProperties
  []
[]

[Problem]
  type = LiquidMetalSubChannel1PhaseProblem
  fp = sodium
  n_blocks = 1
  P_out = 2.0e5
  CT = 2.6
  compute_density = true
  compute_viscosity = true
  compute_power = true
  P_tol = 1.0e-4
  T_tol = 1.0e-4
  implicit = true
  segregated = false
  staggered_pressure = false
  monolithic_thermal = false
  verbose_multiapps = true
  verbose_subchannel = true
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
    power = 5000.0 #W
    axial_heat_rate = axial_heat_rate
    filename = "power_profile.txt"
  []

  [T_ic]
    type = ConstantIC
    variable = T
    value = ${T_in}
  []

  [Dpin_ic]
    type = ConstantIC
    variable = Dpin
    value = 5.84e-3
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

# Need to have as many points as pins in the object
[UserObjects]
  [Tpin_avg_uo]
    type = NearestPointLayeredAverage
    direction = z
    num_layers = 1000
    variable = Tpin
    block = fuel_pins
    points = ' 0   0   0
              ${fparse  1.0 * pitch}   0  0
              ${fparse -1.0 * pitch}   0  0
              ${fparse  0.5 * pitch}   ${fparse  0.866 * pitch}   0
              ${fparse  0.5 * pitch}   ${fparse -0.866 * pitch}   0
              ${fparse -0.5 * pitch}   ${fparse  0.866 * pitch}   0
              ${fparse -0.5 * pitch}   ${fparse -0.866 * pitch}   0'
    execute_on = timestep_end
  []
[]

# [Executioner]
#   type = Steady
# []

# ##############################################################################
# ###  A multiapp that projects data to a detailed mesh
# ##############################################################################

# [MultiApps]
#   [viz]
#     type = FullSolveMultiApp
#     input_files = "3d.i"
#     execute_on = "timestep_end"
#   []
# []

# [Transfers]
#   [subchannel_transfer]
#     type = MultiAppDetailedSolutionTransfer
#     to_multi_app = viz
#     variable = 'mdot SumWij P DP h T rho mu S'
#   []
#   [pin_transfer]
#     type = MultiAppDetailedPinSolutionTransfer
#     to_multi_app = viz
#     variable = 'Tpin q_prime Dpin'
#   []
# []

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  fixed_point_max_its = 2
  fixed_point_min_its = 2
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
    type = MultiAppUserObjectTransfer2
    to_multi_app = sub
    variable = Pin_surface_temperature
    user_object = Tpin_avg_uo
  []

  [diameter] # send diameter information from /BISON/heatConduction to subchannel/master
    type = MultiAppUserObjectTransfer2
    from_multi_app = sub
    variable = Dpin
    user_object = rod_diameter_uo
  []

  [q_prime] # send heat flux from /BISON/heatConduction to subchannel/master
    type = MultiAppUserObjectTransfer2
    from_multi_app = sub
    variable = q_prime
    user_object = q_prime_uo
  []

  [subchannel_transfer]
    type = MultiAppDetailedSolutionTransfer
    to_multi_app = viz
    variable = 'mdot SumWij P DP h T rho mu S'
  []

  [pin_transfer]
    type = MultiAppDetailedPinSolutionTransfer
    to_multi_app = viz
    variable = 'Tpin Dpin q_prime'
  []
[]
