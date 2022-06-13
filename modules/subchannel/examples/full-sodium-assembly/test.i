T_in = 660
# [1e+6 kg/m^2-hour] turns into kg/m^2-sec
mass_flux_in = ${fparse 1e+6 * 37.00 / 36000.*0.5}
P_out = 2.0e5 # Pa
[TriSubChannelMesh]
  [subchannel]
    type = TriSubChannelMeshGenerator
    nrings = 4
    n_cells = 100
    flat_to_flat = 0.085
    heated_length = 1.0
    rod_diameter = 0.01
    pitch = 0.012
    dwire = 0.002
    hwire = 0.0833
    spacer_z = '0 0.2 0.4 0.6 0.8'
    spacer_k = '0.1 0.1 0.1 0.1 0.10'
    discretization = "central_difference"
  []

  [fuel_pins]
    type = TriPinMeshGenerator
    input = subchannel
    nrings = 4
    n_cells = 100
    heated_length = 1.0
    pitch = 0.012
  []

  [duct]
    type = TriDuctMeshGenerator
    input = fuel_pins
    nrings = 4
    n_cells = 100
    flat_to_flat = 0.085
    heated_length = 1.0
    pitch = 0.012
  []
[]

[Functions]
  [axial_heat_rate]
    type = ParsedFunction
    value = '(pi/2)*sin(pi*z/L)'
    vars = 'L'
    vals = '1.0'
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
  [Tpin]
    block = fuel_pins
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
  [q_prime]
    block = fuel_pins
  []
  [mu]
    block = subchannel
  []
  [q_prime_duct]
    block = duct
  []
  [Tduct]
    block = duct
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
  n_blocks = 50
  beta = 0.1
  P_out = 2.0e5
  CT = 1.0
  enforce_uniform_pressure = false
  compute_density = false
  compute_viscosity = false
  compute_power = true
  P_tol = 1.0e-2
  T_tol = 1.0e-2
  implicit = false
  segregated = true
  staggered_pressure = false
  monolithic_thermal = false
  verbose_multiapps = true
  verbose_subchannel = false
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
    power = 1.000e5 # W
    filename = "pin_power_profile37.txt"
    axial_heat_rate = axial_heat_rate
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

  [T_duct_ic]
    type = ConstantIC
    variable = Tduct
    value = ${T_in}
  []
[]

[AuxKernels]
  [P_out_bc]
    type = ConstantAux
    variable = P
    boundary = outlet
    value = ${P_out}
    execute_on = 'timestep_begin'
    block = subchannel
  []
  [T_in_bc]
    type = ConstantAux
    variable = T
    boundary = inlet
    value = ${T_in}
    execute_on = 'timestep_begin'
    block = subchannel
  []
  [mdot_in_bc]
    type = MassFlowRateAux
    variable = mdot
    boundary = inlet
    area = S
    mass_flux = ${mass_flux_in}
    execute_on = 'timestep_begin'
    block = subchannel
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
    execute_on = 'timestep_end'
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
  fixed_point_min_its = 1
  fixed_point_rel_tol = 1e-6
[]

################################################################################
# A multiapp that projects data to a detailed mesh
################################################################################
[MultiApps]
  # Multiapp to duct heat conduction module
  [duct_map]
    type = FullSolveMultiApp
    input_files = wrapper.i # seperate file for duct heat conduction
    execute_on = 'timestep_end'
    positions = '0   0   0'
    bounding_box_padding = '0.0 0.0 0.1'
  []

  # Multiapp to pin heat conduction module
  [pin_map]
    type = FullSolveMultiApp
    input_files = pin.i # seperate file for multiapps due to radial power profile
    execute_on = 'timestep_end'
    positions = '0   0   0'
    bounding_box_padding = '0 0 0.01'
  []


  [viz]
    type = FullSolveMultiApp
    input_files = "3d.i"
    execute_on = "final"
  []
[]

[Transfers]

  [duct_temperature_transfer] # Send duct temperature to heat conduction
    type = MultiAppInterpolationTransfer
    to_multi_app = duct_map
    source_variable = Tduct
    variable = duct_surface_temperature
  []
  [q_prime_duct] # Recover q_prime from heat conduction solve
    type = MultiAppInterpolationTransfer
    from_multi_app = duct_map
    source_variable = q_prime_d
    variable = q_prime_duct
  []

  [Tpin] # send pin surface temperature to bison,
    type = MultiAppUserObjectTransfer2
    to_multi_app = pin_map
    variable = Pin_surface_temperature
    user_object = Tpin_avg_uo
  []
  [q_prime_pin] # send heat flux from slave/BISON/heatConduction to subchannel/master
    type = MultiAppUserObjectTransfer2
    from_multi_app = pin_map
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
    variable = 'Tpin q_prime'
  []
[]
