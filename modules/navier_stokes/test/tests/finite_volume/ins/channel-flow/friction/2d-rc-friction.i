mu = 1.1
rho = 1.1
advected_interp_method = 'average'
velocity_interp_method = 'rc'
coef_linear = ${fparse 25 / mu}

[GlobalParams]
  rhie_chow_user_object = 'rc'
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolator
    u = vel_x
    v = vel_y
    pressure = pressure
  []
[]

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

[Variables]
  [vel_x]
    type = INSFVVelocityVariable
    initial_condition = 1
  []
  [vel_y]
    type = INSFVVelocityVariable
    initial_condition = 1
  []
  [pressure]
    type = INSFVPressureVariable
  []
[]

[FVKernels]
  inactive = 'u_friction_quad v_friction_quad'
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
  []

  [u_advection]
    type = INSFVMomentumAdvection
    variable = vel_x
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'x'
  []
  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_x
    mu = ${mu}
    momentum_component = 'x'
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = vel_x
    momentum_component = 'x'
    pressure = pressure
  []
  [u_friction_linear]
    type = PINSFVMomentumFriction
    variable = vel_x
    Darcy_name = friction_coefficient_linear
    momentum_component = 'x'
    rho = ${rho}
    mu = ${mu}
  []
  [u_friction_quad]
    type = PINSFVMomentumFriction
    variable = vel_x
    Forchheimer_name = friction_coefficient_quad
    momentum_component = 'x'
    rho = ${rho}
    speed = speed
  []

  [v_advection]
    type = INSFVMomentumAdvection
    variable = vel_y
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'y'
  []
  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_y
    mu = ${mu}
    momentum_component = 'y'
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = vel_y
    momentum_component = 'y'
    pressure = pressure
  []
  [v_friction_linear]
    type = PINSFVMomentumFriction
    variable = vel_y
    Darcy_name = friction_coefficient_linear
    momentum_component = 'y'
    rho = ${rho}
    mu = ${mu}
  []
  [v_friction_quad]
    type = PINSFVMomentumFriction
    variable = vel_y
    Forchheimer_name = friction_coefficient_quad
    momentum_component = 'y'
    rho = ${rho}
    speed = speed
  []
[]

[FVBCs]
  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = vel_x
    function = '1'
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = vel_y
    function = '0'
  []
  [walls-u]
    type = INSFVNoSlipWallBC
    boundary = 'top bottom'
    variable = vel_x
    function = 0
  []
  [walls-v]
    type = INSFVNoSlipWallBC
    boundary = 'top bottom'
    variable = vel_y
    function = 0
  []
  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'right'
    variable = pressure
    function = '0'
  []
[]

[FunctorMaterials]
  # Have material friction factor properties compatible with the PINSFVMomentumFriction formulation and
  # backwards compatible with the INSFVMomentumFriction formulation
  inactive = friction_coefficient_exp
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
    prop_names = 'friction_coefficient_quad'
    prop_values = 'friction_coefficient_exp_x friction_coefficient_exp_y 0.0'
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
