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
    ix = '${nx} ${nx/2} ${nx}'
    iy = '${ny} ${ny} ${ny}'
    subdomain_id = '3 1 1
                    2 1 2
                    1 1 1'
  []

  [remove_walls]
    type = BlockDeletionGenerator
    input = cmg
    block = 2
  []

  # Add inlets and outlets
  [top_left]
    type = ParsedGenerateSideset
    input = remove_walls
    combinatorial_geometry = 'x<0.001 & y>1.2'
    new_sideset_name = top_left
  []
  [bottom_left]
    type = ParsedGenerateSideset
    input = top_left
    combinatorial_geometry = 'x<0.001 & y<1.2'
    new_sideset_name = bottom_left
  []
  [top_right]
    type = ParsedGenerateSideset
    input = bottom_left
    combinatorial_geometry = 'x>2.299 & y>1.2'
    new_sideset_name = top_right
  []
  [bottom_right]
    type = ParsedGenerateSideset
    input = top_right
    combinatorial_geometry = 'x>2.299 & y<1.2'
    new_sideset_name = bottom_right
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

    inlet_boundaries = 'left'
    momentum_inlet_types = 'fixed-velocity'
    momentum_inlet_function = '1 0'

    wall_boundaries = 'top bottom'
    momentum_wall_types = 'noslip noslip'

    outlet_boundaries = 'right'
    momentum_outlet_types = 'fixed-pressure'
    pressure_function = '1'

    use_friction_correction = true
    consistent_scaling = 10
    friction_blocks = '1; 3'
    friction_types = 'darcy forchheimer; darcy forchheimer'
    # Base friction
    # friction_coeffs = 'Darcy Forchheimer; Darcy Forchheimer'
    # Combined with diode
    friction_coeffs = 'combined_linear combined_quadratic; combined_linear combined_quadratic'
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
    direction = '1 0 0'
    additional_linear_resistance = '10 0 0' #'100 10 20'
    additional_quadratic_resistance = '10 0 0' #'10 4 4'
    base_linear_friction_coefs = 'Darcy'
    base_quadratic_friction_coefs = 'Forchheimer'
    sum_linear_friction_name = 'diode_linear'
    sum_quadratic_friction_name = 'diode_quad'
    block = '3'
    turn_on_diode = true
  []
  [combine_linear_friction]
    type = ADPiecewiseByBlockVectorFunctorMaterial
    prop_name = 'combined_linear'
    subdomain_to_prop_value = '1 Darcy
                               3 diode_linear'
  []
  [combine_quadratic_friction]
    type = ADPiecewiseByBlockVectorFunctorMaterial
    prop_name = 'combined_quadratic'
    subdomain_to_prop_value = '1 Forchheimer
                               3 diode_quad'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_shift_type -ksp_gmres_restart'
  petsc_options_value = 'lu       NONZERO               200'
  line_search = 'none'

  end_time = 10
  nl_abs_tol = 1e-14
[]

[Controls]
    # Case 1: Diode turns on and blocks (adds friction) flow at a given time


    # Case 2: Diode looks at pressure drop, prevents flow in one direction


    # Case 3: Diode looks at flow direction, prevents flow in one direction
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
    type = PressureDropPostprocessor
    upstream_boundary = 'top_left'
    downstream_boundary = 'top_right'
    weighting_functor = 'momentum' 
  []
  [pdrop_bottom_channel]
    type = PressureDropPostprocessor
    upstream_boundary = 'bottom_left'
    downstream_boundary = 'bottom_right'
    weighting_functor = 'momentum' 
  []

  # Diode operation
  [pdrop_diode]
    type = PressureDropPostprocessor
    upstream_boundary = 'top_left'
    downstream_boundary = 'top_right'
    weighting_functor = 'momentum' 
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
