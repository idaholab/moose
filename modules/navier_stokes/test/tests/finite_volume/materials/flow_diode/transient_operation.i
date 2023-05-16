# Horizontal H junction with flow in different directions in the two branches
# One of the branches has a diode against the direction of the flow that can
# be triggered using the Controls
# There are 3 different strategies available for the diode blocking the flow
# - based on a time trigger
# - based on a pressure drop (here chosen across the diode)
# - based on a mass flow rate (here chosen through the diode)

mu = 0.1
rho = 10

nx = 10
ny = 5

[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 0.3 1'
    dy = '0.5 0.2 0.5'
    ix = '${nx} ${fparse nx/2} ${nx}'
    iy = '${ny} ${ny} ${ny}'
    subdomain_id = '1 1 1
                    2 1 2
                    3 4 1'
  []

  [add_walls]
    type = SideSetsBetweenSubdomainsGenerator
    input = 'cmg'
    primary_block = '1 3 4'
    paired_block = '2'
    new_boundary = 'walls'
  []
  [remove_wall_blocks]
    type = BlockDeletionGenerator
    input = add_walls
    block = 2
  []

  # Add inlets and outlets
  [top_left]
    type = ParsedGenerateSideset
    input = remove_wall_blocks
    combinatorial_geometry = 'x<0.001 & y>0.6'
    new_sideset_name = top_left
  []
  [bottom_left]
    type = ParsedGenerateSideset
    input = top_left
    combinatorial_geometry = 'x<0.001 & y<0.6'
    new_sideset_name = bottom_left
  []
  [top_right]
    type = ParsedGenerateSideset
    input = bottom_left
    combinatorial_geometry = 'x>2.299 & y>0.6'
    new_sideset_name = top_right
  []
  [bottom_right]
    type = ParsedGenerateSideset
    input = top_right
    combinatorial_geometry = 'x>2.299 & y<0.6'
    new_sideset_name = bottom_right
  []

  # Extra surfaces
  [diode_inlet]
    type = SideSetsBetweenSubdomainsGenerator
    input = bottom_right
    primary_block = 4
    paired_block = 3
    new_boundary = 'diode_inlet'
  []
  [mid_section]
    type = SideSetsBetweenSubdomainsGenerator
    input = diode_inlet
    primary_block = 4
    paired_block = 1
    new_boundary = 'mid_connection'
  []

  [reduce_blocks]
    type = RenameBlockGenerator
    input = 'mid_section'
    old_block = '4 3 1'
    new_block = '1 diode fluid'
  []
[]

[GlobalParams]
  rhie_chow_user_object = 'pins_rhie_chow_interpolator'
  advected_interp_method = 'upwind'
  velocity_interp_method = 'rc'
[]

[Modules]
  [NavierStokesFV]
    compressibility = 'incompressible'
    porous_medium_treatment = true

    density = ${rho}
    dynamic_viscosity = ${mu}

    initial_velocity = '1e-6 1e-6 0'
    initial_pressure = 0.0

    inlet_boundaries = 'bottom_left top_right'
    momentum_inlet_types = 'fixed-velocity fixed-velocity'
    momentum_inlet_function = '1 0; -1 0'

    wall_boundaries = 'top bottom walls'
    momentum_wall_types = 'noslip noslip noslip'

    outlet_boundaries = 'bottom_right top_left'
    momentum_outlet_types = 'fixed-pressure fixed-pressure'
    pressure_function = '1 1'

    friction_blocks = 'fluid; diode'
    friction_types = 'darcy forchheimer; darcy forchheimer'
    # Base friction
    # friction_coeffs = 'Darcy Forchheimer; Darcy Forchheimer'
    # Combined with diode
    friction_coeffs = 'combined_linear combined_quadratic; combined_linear combined_quadratic'

    # Porosity jump treatment
    # Option 1: diffusion correction
    use_friction_correction = true
    consistent_scaling = 10

    # Option 2: bernouilli jump
    # porosity_interface_pressure_treatment = bernoulli

    mass_advection_interpolation = 'average'
    momentum_advection_interpolation = 'average'
  []
[]

[Materials]
  [porosity]
    type = ADGenericFunctorMaterial
    prop_names = 'porosity'
    prop_values = '0.5'
  []
  [base_friction]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'Darcy Forchheimer'
    prop_values = '1 1 1 0.1 0.2 0.3'
  []

  # Material definitions needed for the diode
  [diode]
    type = NSFVFrictionFlowDiodeMaterial
    # Friction only in X direction
    direction = '-1 0 0'
    additional_linear_resistance = '100 0 0'
    additional_quadratic_resistance = '0 0 0'
    base_linear_friction_coefs = 'Darcy'
    base_quadratic_friction_coefs = 'Forchheimer'
    sum_linear_friction_name = 'diode_linear'
    sum_quadratic_friction_name = 'diode_quad'
    block = 'diode'
    turn_on_diode = false
  []
  [combine_linear_friction]
    type = ADPiecewiseByBlockVectorFunctorMaterial
    prop_name = 'combined_linear'
    subdomain_to_prop_value = 'fluid Darcy
                               diode diode_linear'
  []
  [combine_quadratic_friction]
    type = ADPiecewiseByBlockVectorFunctorMaterial
    prop_name = 'combined_quadratic'
    subdomain_to_prop_value = 'fluid Forchheimer
                               diode diode_quad'
  []

  # density is constant
  [momentum]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'momentum'
    prop_values = 'superficial_vel_x superficial_vel_y 0'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_shift_type -ksp_gmres_restart'
  petsc_options_value = 'lu       NONZERO               200'
  line_search = 'none'

  end_time = 0.2
  dt = 0.015
  nl_abs_tol = 1e-12
[]

[Controls]
  active = 'pdrop_based'
  # Case 1: Diode turns on at a certain time and blocks (adds friction) flow at a given time
  [time_based]
    type = BoolFunctionControl
    function = time_function
    parameter = 'Materials/diode/turn_on_diode'
    execute_on = timestep_begin
  []

  # Case 2: Diode looks at pressure drop, reduces flow if positive pressure drop
  # This will not oscillate as the diode increases the pressure drop
  [pdrop_based]
    type = BoolFunctionControl
    function = pdrop_positive
    parameter = 'Materials/diode/turn_on_diode'
    execute_on = timestep_begin
  []

  # Case 3: Diode looks at flow direction & quantity, reduces flow if too much flow
  # in a given direction
  # This will oscillate (turn on/off on each step) if the action of turning the diode
  # makes the amount of flow smaller than the threshold for turning on the diode
  [flow_based]
    type = BoolFunctionControl
    function = velocity_big_enough
    parameter = 'Materials/diode/turn_on_diode'
    execute_on = timestep_begin
  []
[]

[Functions]
  # Functions are used to parse postprocessors and provide them to a BoolFunctionControl
  [time_function]
    type = ParsedFunction
    expression = 'if(t<0.1, 0, 1)'
  []
  [pdrop_positive]
    type = ParsedFunction
    expression = 'if(pdrop_diode>100, 1, 0)'
    symbol_names = pdrop_diode
    symbol_values = pdrop_diode
  []
  [velocity_big_enough]
    type = ParsedFunction
    expression = 'if(flow_diode<-0.4, 1, 0)'
    symbol_names = flow_diode
    symbol_values = flow_diode
  []
[]

[Postprocessors]
  # Analysis of the simulation
  [mdot_top]
    type = VolumetricFlowRate
    boundary = 'top_right'
    vel_x = superficial_vel_x
    vel_y = superficial_vel_y
    advected_quantity = ${rho}
  []
  [mdot_bottom]
    type = VolumetricFlowRate
    boundary = 'bottom_right'
    vel_x = superficial_vel_x
    vel_y = superficial_vel_y
    advected_quantity = ${rho}
  []
  [mdot_middle]
    type = VolumetricFlowRate
    boundary = 'mid_connection'
    vel_x = superficial_vel_x
    vel_y = superficial_vel_y
    advected_quantity = ${rho}
  []
  [pdrop_top_channel]
    type = PressureDrop
    upstream_boundary = 'top_left'
    downstream_boundary = 'top_right'
    weighting_functor = 'momentum'
    boundary = 'top_left top_right'
    pressure = pressure
  []
  [pdrop_bottom_channel]
    type = PressureDrop
    upstream_boundary = 'bottom_left'
    downstream_boundary = 'bottom_right'
    weighting_functor = 'momentum'
    boundary = 'bottom_left bottom_right'
    pressure = pressure
  []

  # Diode operation
  [pdrop_diode]
    type = PressureDrop
    upstream_boundary = 'diode_inlet'
    downstream_boundary = 'top_left'
    weighting_functor = 'momentum'
    boundary = 'diode_inlet top_left'
    pressure = pressure
  []
  [flow_diode]
    type = VolumetricFlowRate
    boundary = 'diode_inlet'
    vel_x = superficial_vel_x
    vel_y = superficial_vel_y
    advected_quantity = ${rho}
  []
[]

[Outputs]
  exodus = true
  csv = true
[]
