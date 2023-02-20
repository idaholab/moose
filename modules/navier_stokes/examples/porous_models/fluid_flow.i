length = 25e-6 #m
D = 5e-6 #m
rho_left = 2300 #kg/m3
mu_left = 0.005 #Pa.s
rho_right = 1.0 #kg/m3
mu_right = 0.00001 #Pa.s
darcy_blockage = 1e12

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = ${length}
    ymin = 0
    ymax = ${D}
    nx = 50
    ny = 10
  []
  [rename1]
    type = RenameBoundaryGenerator
    input = gen
    old_boundary = 'left'
    new_boundary = 'inlet'
  []
  [rename2]
    type = RenameBoundaryGenerator
    input = rename1
    old_boundary = 'right'
    new_boundary = 'outlet'
  []
  [rename3]
    type = RenameBoundaryGenerator
    input = rename2
    old_boundary = 'bottom'
    new_boundary = 'wall_bottom'
  []
  [rename4]
    type = RenameBoundaryGenerator
    input = rename3
    old_boundary = 'top'
    new_boundary = 'wall_top'
  []
[]

[Problem]
  kernel_coverage_check = false
  fv_bcs_integrity_check = true
[]

[GlobalParams]
  rhie_chow_user_object = 'rc'
  advected_interp_method = 'upwind'
  velocity_interp_method = 'rc'
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolator
    u = u
    v = v
    pressure = pressure
  []
[]

[Variables]
  [u]
    type = INSFVVelocityVariable
    initial_condition = 0.0
  []
  [v]
    type = INSFVVelocityVariable
    initial_condition = 0.0
  []
  [pressure]
    type = INSFVPressureVariable
  []
[]

[FVKernels]
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    rho = 'rho'
  []

  [u_time]
    type = INSFVMomentumTimeDerivative
    variable = u
    rho = 'rho'
    momentum_component = 'x'
  []
  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = u
    mu = 'mu'
    momentum_component = 'x'
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = u
    momentum_component = 'x'
    pressure = pressure
  []
  [u_body]
    type = FVCoupledForce
    variable = u
    v = body_force_x
    coef = -1e-10
  []
  # [u_friction]
  #   type = INSFVMomentumFriction
  #   variable = u
  #   momentum_component = 'x'
  #   linear_coef_name = 'friction_coefficient'
  # []

  [v_time]
    type = INSFVMomentumTimeDerivative
    variable = v
    rho = 'rho'
    momentum_component = 'y'
  []
  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = v
    mu = 'mu'
    momentum_component = 'y'
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = v
    momentum_component = 'y'
    pressure = pressure
  []
  # [v_body]
  #   type = FVCoupledForce
  #   variable = v
  #   v = computed_phi_grad_y
  #   coef = -1
  # []
[]

[AuxVariables]
  [phi]
    type = MooseVariableFVReal
  []
  [phi_grad]
    type = VectorMooseVariable
    family = MONOMIAL_VEC
    order = CONSTANT
  []
  [computed_phi_grad_x]
    type = MooseVariableFVReal
  []
  [computed_phi_grad_y]
    type = MooseVariableFVReal
  []
  [darcy_field]
    type = MooseVariableFVReal
  []
  [rho_field]
    type = MooseVariableFVReal
  []
  [mu_field]
    type = MooseVariableFVReal
  []
  [rho_grad]
    type = VectorMooseVariable
    family = MONOMIAL_VEC
    order = CONSTANT
  []
  [computed_rho_grad_x]
    type = MooseVariableFVReal
  []
  [computed_rho_grad_y]
    type = MooseVariableFVReal
  []
  [body_force_x]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [populate_phi]
    type = ParsedAux
    variable = phi
    constant_names = 'D length'
    constant_expressions = '${D} ${length}'
    use_xyzt = true
    expression = '1.0 - (y * (D - y))/(D*D) - 0.5*(x/length)'
  []
  [populate_phi_grad]
    type = ADFunctorElementalGradientAux
    variable = phi_grad
    functor = phi
  []
  [populate_phi_grad_x]
    type = VectorVariableComponentAux
    variable = computed_phi_grad_x
    vector_variable = phi_grad
    component = 'x'
  []
  [populate_phi_grad_y]
    type = VectorVariableComponentAux
    variable = computed_phi_grad_y
    vector_variable = phi_grad
    component = 'y'
  []
  [populate_darcy]
    type = ADFunctorElementalAux
    variable = darcy_field
    functor = darcy_coefs_function
  []
  [populate_density]
    type = ADFunctorElementalAux
    variable = rho_field
    functor = step_density
  []
  [populate_viscosity]
    type = ADFunctorElementalAux
    variable = mu_field
    functor = step_viscosity
  []
  [compute_gradient_density]
    type = ADFunctorElementalGradientAux
    variable = rho_grad
    functor = rho_field
  []
  [populate_rho_grad_x]
    type = VectorVariableComponentAux
    variable = computed_rho_grad_x
    vector_variable = rho_grad
    component = 'x'
  []
  [populate_rho_grad_y]
    type = VectorVariableComponentAux
    variable = computed_rho_grad_y
    vector_variable = rho_grad
    component = 'y'
  []
  [populate_body_force_x]
    type = ParsedAux
    variable = body_force_x
    coupled_variables = 'computed_phi_grad_x computed_rho_grad_x'
    expression = 'computed_phi_grad_x * computed_rho_grad_x'
  []
[]

[Functions]
  [darcy_coefs_function]
    type = ADParsedFunction
    symbol_names = 'factor D length'
    symbol_values = '${darcy_blockage} ${D} ${length}'
    expression = 'if((x < length / 3) | (x > 2 * length / 3) | (y < D/2), 0.0, factor)'
    execute_on = 'INITIAL'
  []
  [step_density]
    type = ADParsedFunction
    symbol_names = 'rho_left rho_right length'
    symbol_values = '${rho_left} ${rho_right} ${length}'
    expression = 'if((x <= length / 2), rho_left, rho_right)'
    execute_on = 'INITIAL'
  []
  [step_viscosity]
    type = ADParsedFunction
    symbol_names = 'mu_left mu_right length'
    symbol_values = '${mu_left} ${mu_right} ${length}'
    expression = 'if((x <= length / 2), mu_left, mu_right)'
    execute_on = 'INITIAL'
  []
[]

[Materials]
  [friction_coefficient]
    type = ADGenericFunctorMaterial
    prop_names = 'friction_coefficient rho mu'
    prop_values = 'darcy_coefs_function step_density step_viscosity'
  []
  # [friction_coefficient]
  #   type = ADGenericFunctorMaterial
  #   prop_names = 'friction_coefficient'
  #   prop_values = '25'
  # []
[]

[FVBCs]
  # [inlet_u]
  #   type = INSFVInletVelocityBC
  #   boundary = 'inlet'
  #   variable = u
  #   function = 1e-6
  # []
  # [inlet_v]
  #   type = INSFVInletVelocityBC
  #   boundary = 'inlet'
  #   variable = v
  #   function = '0'
  # []
  [walls_u]
    type = INSFVNoSlipWallBC
    boundary = 'wall_bottom wall_top'
    variable = u
    function = 0
  []
  [walls_v]
    type = INSFVNoSlipWallBC
    boundary = 'wall_bottom wall_top'
    variable = v
    function = 0
  []
  [inlet_p]
    type = INSFVOutletPressureBC
    boundary = 'inlet'
    variable = pressure
    function = '0'
  []
  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'outlet'
    variable = pressure
    function = '0'
  []
[]

[Executioner]
  type = Transient
  dt = 0.1
  end_time = 1.0
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  line_search = 'none'
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10
[]

[Outputs]
  exodus = true
[]

