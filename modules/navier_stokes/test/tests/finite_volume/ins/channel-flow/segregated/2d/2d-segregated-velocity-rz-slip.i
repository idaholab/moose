mu = 2.6
rho = 1.0
advected_interp_method = 'average'
velocity_interp_method = 'rc'
Darcy_coef = ${fparse 0.1 / mu}

pressure_tag = "pressure_grad"

[Mesh]
  coord_type = 'RZ'
  rz_coord_axis = X
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1.25'
    dy = '0.2'
    ix = '30'
    iy = '7'
  []
[]

[GlobalParams]
  rhie_chow_user_object = 'rc'
[]

[Problem]
  nl_sys_names = 'u_system v_system pressure_system'
  previous_nl_solution_required = true
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolatorSegregated
    u = vel_x
    v = vel_y
    pressure = pressure
  []
[]

[Variables]
  [vel_x]
    type = INSFVVelocityVariable
    initial_condition = 0.5
    nl_sys = u_system
    two_term_boundary_expansion = false
  []
  [vel_y]
    type = INSFVVelocityVariable
    initial_condition = 0.0
    nl_sys = v_system
    two_term_boundary_expansion = false
  []
  [pressure]
    type = INSFVPressureVariable
    nl_sys = pressure_system
    initial_condition = 0.2
    two_term_boundary_expansion = false
  []
[]

[FVKernels]
  inactive = 'u_friction v_friction'
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
    extra_vector_tags = ${pressure_tag}
  []
  [u_friction]
    type = PINSFVMomentumFriction
    variable = vel_x
    momentum_component = 'y'
    Darcy_name = 'Darcy_coefficient'
    Forchheimer_name = 'Forchheimer_coefficient'
    rho = ${rho}
    speed = speed
    mu = ${mu}
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
    extra_vector_tags = ${pressure_tag}
  []
  [v_friction]
    type = PINSFVMomentumFriction
    variable = vel_y
    momentum_component = 'y'
    Darcy_name = 'Darcy_coefficient'
    Forchheimer_name = 'Forchheimer_coefficient'
    rho = ${rho}
    speed = speed
    mu = ${mu}
  []
  [p_diffusion]
    type = FVAnisotropicDiffusion
    variable = pressure
    coeff = "Ainv"
    coeff_interp_method = 'average'
  []
  [p_source]
    type = FVDivergence
    variable = pressure
    vector_field = "HbyA"
    force_boundary_execution = true
  []
[]

[FVBCs]
  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = vel_x
    function = '1.1'
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = vel_y
    function = '0.0'
  []
  [walls-u]
    type = INSFVNaturalFreeSlipBC
    boundary = 'top'
    variable = vel_x
    momentum_component = 'x'
  []
  [walls-v]
    type = INSFVNaturalFreeSlipBC
    boundary = 'top'
    variable = vel_y
    momentum_component = 'y'
  []
  [symmetry_u]
    type = INSFVSymmetryVelocityBC
    variable = vel_x
    boundary = 'bottom'
    momentum_component = 'x'
    mu = ${mu}
    u = vel_x
    v = vel_y
  []
  [symmetry_v]
    type = INSFVSymmetryVelocityBC
    variable = vel_y
    boundary = 'bottom'
    momentum_component = 'y'
    mu = ${mu}
    u = vel_x
    v = vel_y
  []
  [symmetry_pressure]
    type = INSFVSymmetryPressureBC
    boundary = 'bottom'
    variable = pressure
  []
  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'right'
    variable = pressure
    function = 1.4
  []
[]

[FunctorMaterials]
  # Have material friction factor properties compatible with the PINSFVMomentumFriction formulation and
  # backwards compatible with the INSFVMomentumFriction formulation
  [friction_coefficient_quad_x]
    type = ADParsedFunctorMaterial
    functor_names = 'speed vel_x'
    property_name = 'friction_coefficient_quad_x'
    expression = '2.0 * 0.1 * abs(vel_x) / ${rho} / speed'
  []
  [friction_coefficient_quad_y]
    type = ADParsedFunctorMaterial
    functor_names = 'speed vel_y'
    property_name = 'friction_coefficient_quad_y'
    expression = '2.0 * 0.1 * abs(vel_y) / ${rho} / speed'
  []
  [friction_coefficient_quad]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'Forchheimer_coefficient'
    prop_values = 'friction_coefficient_quad_x friction_coefficient_quad_y 0.0'
  []
  [friction_coefficient_linear]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'Darcy_coefficient'
    prop_values = '${Darcy_coef} ${Darcy_coef} ${Darcy_coef}'
  []
  [speed_material]
    type = PINSFVSpeedFunctorMaterial
    superficial_vel_x = vel_x
    superficial_vel_y = vel_y
    porosity = 1
    vel_x = vel_x_mat
    vel_y = vel_y_mat
  []
[]

[Executioner]
  type = SIMPLE
  momentum_l_abs_tol = 1e-14
  pressure_l_abs_tol = 1e-14
  momentum_l_tol = 0
  pressure_l_tol = 0
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  pressure_gradient_tag = ${pressure_tag}
  momentum_equation_relaxation = 0.5
  pressure_variable_relaxation = 0.3
  num_iterations = 150
  pressure_absolute_tolerance = 1e-13
  momentum_absolute_tolerance = 1e-13
  print_fields = false
[]

[Outputs]
  exodus = true
  csv = false
  perf_graph = false
  print_nonlinear_residuals = false
  print_linear_residuals = true
[]
