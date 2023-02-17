T_in = 588.5
flow_area = 0.0004980799633447909 #m2
mass_flux_in = ${fparse 55*3.78541/10/60/flow_area}
P_out = 2.0e5 # Pa
heated_length = 1
[TriSubChannelMesh]
  [subchannel]
    type = TriSubChannelMeshGenerator
    nrings = 3
    n_cells = 25
    flat_to_flat = 3.41e-2
    heated_length = 1
    rod_diameter = 5.84e-3
    pitch = 7.26e-3
    dwire = 1.42e-3
    hwire = 0.3048
    spacer_z = '0'
    spacer_k = '0'
  []

  [fuel_pins]
    type = TriPinMeshGenerator
    input = subchannel
    nrings = 3
    n_cells = 25
    heated_length = 1
    pitch = 7.26e-3
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
[]

[Functions]
    [axial_heat_rate]
        type = ParsedFunction
        value = '(pi/2)*sin(pi*z/L)'
        vars = 'L'
        vals = '${heated_length}'
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
  beta = 0.006
  P_out = 2.0e5
  CT = 2.6
  # enforce_uniform_pressure = false
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
  verbose_subchannel = false
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
    power = 19000 #${fparse 16975/(0.5334+0.4046+0.0762)} # W/m
    filename = "pin_power_profile19.txt"
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

[Outputs]
  exodus = true
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
# MultiApps
################################################################################

[MultiApps]
[sub]
    app_type = BisonApp
    type = FullSolveMultiApp
    input_files = 'sub.i'
    execute_on = 'timestep_end'
    positions = '0 0 0'
    output_in_position = true
    bounding_box_padding = '0 0 0.1'
[]
#   [viz]
#     type = FullSolveMultiApp
#     input_files = "3d_ORNL_19.i"
#     execute_on = "timestep_end"
#   []
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
#   [xfer]
#     type = MultiAppDetailedSolutionTransfer
#     to_multi_app = viz
#     variable = 'mdot SumWij P DP h T rho mu q_prime S'
#   []
[]
