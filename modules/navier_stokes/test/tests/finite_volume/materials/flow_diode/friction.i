mu = 0.1
rho = 10

[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 0.5 1'
    dy = '0.5 0.5'
    ix = '3 2 3'
    iy = '3 3'
    subdomain_id = '1 1 2
                    2 1 1'
  []

  [top_outlet]
    type = ParsedGenerateSideset
    input = cmg
    combinatorial_geometry = 'x>2.499 & y>0.4999'
    new_sideset_name = top_right
  []

  [bottom_outlet]
    type = ParsedGenerateSideset
    input = top_outlet
    combinatorial_geometry = 'x>2.499 & y<0.50001'
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
    friction_blocks = '1; 2'
    friction_types = 'darcy forchheimer; darcy forchheimer'
    # Base friction
    # friction_coeffs = 'Darcy Forchheimer; Darcy Forchheimer'
    # Combined with diode
    friction_coeffs = 'combined_linear combined_quadratic; combined_linear combined_quadratic'

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
  [diode]
    type = NSFVFrictionFlowDiodeMaterial
    direction = '1 0 0'
    additional_linear_resistance = '10 0 0' #'100 10 20'
    additional_quadratic_resistance = '10 0 0' #'10 4 4'
    base_linear_friction_coefs = 'Darcy'
    base_quadratic_friction_coefs = 'Forchheimer'
    sum_linear_friction_name = 'diode_linear'
    sum_quadratic_friction_name = 'diode_quad'
    block = '2'
    turn_on_diode = true
  []
  [combine_linear_friction]
    type = ADPiecewiseByBlockVectorFunctorMaterial
    prop_name = 'combined_linear'
    subdomain_to_prop_value = '1 Darcy
                               2 diode_linear'
  []
  [combine_quadratic_friction]
    type = ADPiecewiseByBlockVectorFunctorMaterial
    prop_name = 'combined_quadratic'
    subdomain_to_prop_value = '1 Forchheimer
                               2 diode_quad'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_shift_type -ksp_gmres_restart'
  petsc_options_value = 'lu       NONZERO               200'
  line_search = 'none'

  nl_abs_tol = 1e-14
[]

[Postprocessors]
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
[]

[Outputs]
  exodus = true
[]
