mu = 1.1
rho = 1.1
coef_linear = ${fparse 25 / mu}

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 5
    ymin = -1
    ymax = 1
    nx = 50
    ny = 10
  []
[]

[Modules]
  [NavierStokesFV]
    compressibility = 'incompressible'

    density = 'rho'
    dynamic_viscosity = 'mu'

    initial_velocity = '1 1 0'
    initial_pressure = 0.0

    inlet_boundaries = 'left'
    momentum_inlet_types = 'fixed-velocity'
    momentum_inlet_function = '1 0'
    wall_boundaries = 'top bottom'
    momentum_wall_types = 'noslip noslip'
    outlet_boundaries = 'right'
    momentum_outlet_types = 'fixed-pressure'
    pressure_function = '0'

    friction_types = 'darcy'
    friction_coeffs = 'friction_coefficient_linear'

    mass_advection_interpolation = 'average'
    momentum_advection_interpolation = 'average'
  []
[]

[FunctorMaterials]
  [friction_coefficient_linear]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'friction_coefficient_linear'
    prop_values = '${coef_linear} ${coef_linear} ${coef_linear}'
  []
  [friction_coefficient_quad_x]
    type = ADParsedFunctorMaterial
    functor_names = 'speed vel_x'
    property_name = 'friction_coefficient_quad_x'
    expression = '2.0 * 25 * abs(vel_x) / ${rho} / speed'
  []
  [friction_coefficient_quad_y]
    type = ADParsedFunctorMaterial
    functor_names = 'speed vel_y'
    property_name = 'friction_coefficient_quad_y'
    expression = '2.0 * 25 * abs(vel_y) / ${rho} / speed'
  []
  [friction_coefficient_quad]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'friction_coefficient_quad'
    prop_values = 'friction_coefficient_quad_x friction_coefficient_quad_y 0.0'
  []
  [friction_coefficient_exp_x]
    type = ADParsedFunctorMaterial
    functor_names = 'speed vel_x coef_exp'
    property_name = 'friction_coefficient_exp_x'
    expression = '2.0 * coef_exp * abs(vel_x) / ${rho} / speed'
  []
  [friction_coefficient_exp_y]
    type = ADParsedFunctorMaterial
    functor_names = 'speed vel_y coef_exp'
    property_name = 'friction_coefficient_exp_y'
    expression = '2.0 * coef_exp * abs(vel_y) / ${rho} / speed'
  []
  [friction_coefficient_exp]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'friction_coefficient_exp'
    prop_values = 'friction_coefficient_exp_x friction_coefficient_exp_y 0.0'
  []
  [const]
    type = ADGenericFunctorMaterial
    prop_names = 'rho mu'
    prop_values = '${rho} ${mu}'
  []
  [speed_material]
    type = PINSFVSpeedFunctorMaterial
    superficial_vel_x = vel_x
    superficial_vel_y = vel_y
    porosity = 1
    vel_x = vel_x_mat
    vel_y = vel_y_mat
  []
  [Re_material]
    type = ReynoldsNumberFunctorMaterial
    speed = speed
    characteristic_length = 2
    rho = ${rho}
    mu = ${mu}
  []
  [coef_exp]
    type = ExponentialFrictionFunctorMaterial
    friction_factor_name = 'coef_exp'
    Re = Re
    c1 = 0.25
    c2 = 0.55
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
