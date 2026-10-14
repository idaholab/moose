# Manufactured pressure-baffle problem with piecewise-linear superficial velocity. In coordinates
# normal (r) and tangent (t) to the interface, rho * U_r is constant and
# U_t = tangential_velocity + tangential_slope_s * r on side s. Thus mass is conserved, while the
# two exact one-sided velocity gradients are known and can differ at the interface. The default
# equal porosity and interpolated-density jump make the exact pressure and its gradient zero for
# both the continuous-density and density-jump variants.

rho_left = 1
rho_right = 1
eps_left = 1
eps_right = 1
mass_flux = 0.02
mu = 0
reaction = 10
tangential_velocity = 0.1
tangential_slope_left = 0.2
tangential_slope_right = 0.2
interface = 0.5
skew = 0
nx = 8
ny = 8
gradient_policy = one-sided
advection_policy = one-sided
pressure_gradient_method = reconstructed
nonorthogonal = false

[Problem]
  linear_sys_names = 'u_system v_system pressure_system'
  previous_nl_solution_required = true
[]

[Mesh]
  [generated]
    type = GeneratedMeshGenerator
    dim = 2
    nx = ${nx}
    ny = ${ny}
  []
  [right]
    type = ParsedSubdomainMeshGenerator
    input = generated
    combinatorial_geometry = 'x > ${interface}'
    block_id = 2
    block_name = right
  []
  [left]
    type = ParsedSubdomainMeshGenerator
    input = right
    combinatorial_geometry = 'x < ${interface}'
    block_id = 1
    block_name = left
  []
  [skew]
    type = ParsedNodeTransformGenerator
    input = left
    x_function = 'x + ${skew} * y'
  []
  [baffle]
    type = SideSetsBetweenSubdomainsGenerator
    input = skew
    primary_block = left
    paired_block = right
    new_boundary = baffle
  []
[]

[FVGradientMethods]
  [jump_aware]
    type = FVPressureJumpGreenGaussGradient
  []
  [reconstructed]
    type = FVReconstructedPressureGradient
    base_gradient_method = jump_aware
    baffle_velocity_gradient_reconstruction = ${gradient_policy}
    gradient_relaxation = 1
  []
[]

[FVInterpolationMethods]
  [average]
    type = FVGeometricAverage
  []
[]

[UserObjects]
  [pressure_jump]
    type = BernoulliFormLossPressureJump
    boundary = baffle
    porosity = porosity
    density = density
    use_interpolated_density = true
  []
  [rc]
    type = PorousRhieChowMassFlux
    u = superficial_u
    v = superficial_v
    pressure = pressure
    rho = density
    porosity = porosity
    p_diffusion_kernel = p_diffusion
    pressure_jump_models = pressure_jump
    pressure_projection_method = consistent
  []
[]

[Variables]
  [superficial_u]
    type = MooseLinearVariableFVReal
    solver_sys = u_system
    initial_condition = ${mass_flux}
  []
  [superficial_v]
    type = MooseLinearVariableFVReal
    solver_sys = v_system
    initial_condition = ${tangential_velocity}
  []
  [pressure]
    type = MooseLinearVariableFVReal
    solver_sys = pressure_system
    initial_condition = 0
    gradient_method = jump_aware
  []
[]

[LinearFVKernels]
  [u_advection]
    type = LinearPWCNSFVMomentumFlux
    variable = superficial_u
    advected_interp_method_name = average
    baffle_advection_reconstruction = ${advection_policy}
    mu = ${mu}
    u = superficial_u
    v = superficial_v
    momentum_component = x
    rhie_chow_user_object = rc
    use_nonorthogonal_correction = ${nonorthogonal}
    use_two_point_stress_transmissibility = true
  []
  [v_advection]
    type = LinearPWCNSFVMomentumFlux
    variable = superficial_v
    advected_interp_method_name = average
    baffle_advection_reconstruction = ${advection_policy}
    mu = ${mu}
    u = superficial_u
    v = superficial_v
    momentum_component = y
    rhie_chow_user_object = rc
    use_nonorthogonal_correction = ${nonorthogonal}
    use_two_point_stress_transmissibility = true
  []
  [u_pressure]
    type = LinearPWCNSFVMomentumPressure
    variable = superficial_u
    pressure = pressure
    porosity = porosity
    momentum_component = x
    gradient_method = ${pressure_gradient_method}
  []
  [v_pressure]
    type = LinearPWCNSFVMomentumPressure
    variable = superficial_v
    pressure = pressure
    porosity = porosity
    momentum_component = y
    gradient_method = ${pressure_gradient_method}
  []
  [u_source]
    type = LinearFVSource
    variable = superficial_u
    source_density = forcing_u
  []
  [u_reaction]
    type = LinearFVReaction
    variable = superficial_u
    coeff = ${reaction}
  []
  [v_source]
    type = LinearFVSource
    variable = superficial_v
    source_density = forcing_v
  []
  [v_reaction]
    type = LinearFVReaction
    variable = superficial_v
    coeff = ${reaction}
  []
  [p_diffusion]
    type = LinearFVPressureCorrectionDiffusionJump
    variable = pressure
    rhie_chow_user_object = rc
    use_nonorthogonal_correction = ${nonorthogonal}
  []
  [HbyA_divergence]
    type = LinearFVDivergence
    variable = pressure
    face_flux = HbyA
    force_boundary_execution = true
  []
[]

[LinearFVBCs]
  [velocity_x]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left right top bottom'
    variable = superficial_u
    functor = exact_u
  []
  [velocity_y]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left right top bottom'
    variable = superficial_v
    functor = exact_v
  []
  [pressure_flux]
    type = LinearFVPressureFluxBC
    boundary = 'left right top bottom'
    variable = pressure
    HbyA_flux = HbyA
    Ainv = Ainv
    rho = density
    u = superficial_u
    v = superficial_v
  []
[]

[FunctorMaterials]
  [density]
    type = ParsedFunctorMaterial
    property_name = density
    expression = 'if(x-${skew}*y<${interface},${rho_left},${rho_right})'
  []
  [porosity]
    type = ParsedFunctorMaterial
    property_name = porosity
    expression = 'if(x-${skew}*y<${interface},${eps_left},${eps_right})'
  []
[]

[AuxVariables]
  [reconstructed_grad_p]
    family = MONOMIAL_VEC
    order = CONSTANT
  []
[]

[AuxKernels]
  [reconstructed_grad_p]
    type = LinearFVGradientAux
    variable = reconstructed_grad_p
    linear_fv_variable = pressure
    gradient_method = reconstructed
    execute_on = FINAL
  []
[]

[Functions]
  [normal_coordinate]
    type = ParsedFunction
    expression = '(x-skew*y-interface)/sqrt(1+skew^2)'
    symbol_names = 'skew interface'
    symbol_values = '${skew} ${interface}'
  []
  [exact_normal_velocity]
    type = ParsedFunction
    expression = 'mass_flux/if(x-skew*y<interface,rho_left,rho_right)'
    symbol_names = 'skew interface mass_flux rho_left rho_right'
    symbol_values = '${skew} ${interface} ${mass_flux} ${rho_left} ${rho_right}'
  []
  [exact_tangential_velocity]
    type = ParsedFunction
    expression = 'velocity+if(x-skew*y<interface,slope_left,slope_right)*r'
    symbol_names = 'skew interface velocity slope_left slope_right r'
    symbol_values = '${skew} ${interface} ${tangential_velocity} ${tangential_slope_left} ${tangential_slope_right} normal_coordinate'
  []
  [exact_u]
    type = ParsedFunction
    expression = '(normal+skew*tangential)/sqrt(1+skew^2)'
    symbol_names = 'skew normal tangential'
    symbol_values = '${skew} exact_normal_velocity exact_tangential_velocity'
  []
  [exact_v]
    type = ParsedFunction
    expression = '(-skew*normal+tangential)/sqrt(1+skew^2)'
    symbol_names = 'skew normal tangential'
    symbol_values = '${skew} exact_normal_velocity exact_tangential_velocity'
  []
  [forcing_u]
    type = ParsedFunction
    expression = 'skew*mass_flux*slope/(porosity*sqrt(1+skew^2))+reaction*u'
    symbol_names = 'skew interface mass_flux slope porosity reaction u'
    symbol_values = '${skew} ${interface} ${mass_flux} tangential_slope exact_porosity ${reaction} exact_u'
  []
  [forcing_v]
    type = ParsedFunction
    expression = 'mass_flux*slope/(porosity*sqrt(1+skew^2))+reaction*v'
    symbol_names = 'skew interface mass_flux slope porosity reaction v'
    symbol_values = '${skew} ${interface} ${mass_flux} tangential_slope exact_porosity ${reaction} exact_v'
  []
  [tangential_slope]
    type = ParsedFunction
    expression = 'if(x-skew*y<interface,slope_left,slope_right)'
    symbol_names = 'skew interface slope_left slope_right'
    symbol_values = '${skew} ${interface} ${tangential_slope_left} ${tangential_slope_right}'
  []
  [exact_porosity]
    type = ParsedFunction
    expression = 'if(x-skew*y<interface,eps_left,eps_right)'
    symbol_names = 'skew interface eps_left eps_right'
    symbol_values = '${skew} ${interface} ${eps_left} ${eps_right}'
  []
  [zero]
    type = ConstantFunction
    value = 0
  []
[]

[Executioner]
  type = SIMPLE
  momentum_l_abs_tol = 1e-12
  pressure_l_abs_tol = 1e-12
  momentum_l_tol = 0
  pressure_l_tol = 0
  momentum_l_max_its = 100
  pressure_l_max_its = 100
  rhie_chow_user_object = rc
  momentum_systems = 'u_system v_system'
  pressure_system = pressure_system
  momentum_equation_relaxation = 0.7
  pressure_variable_relaxation = 0.5
  num_iterations = 1000
  pressure_absolute_tolerance = 1e-11
  momentum_absolute_tolerance = 1e-11
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  print_fields = false

  pin_pressure = true
  pressure_pin_value = 0
  pressure_pin_point = '0.25 0.5 0'
[]

[Postprocessors]
  [h]
    type = AverageElementSize
    execute_on = FINAL
  []
  [L2u]
    type = ElementL2FunctorError
    approximate = superficial_u
    exact = exact_u
    execute_on = FINAL
  []
  [L2v]
    type = ElementL2FunctorError
    approximate = superficial_v
    exact = exact_v
    execute_on = FINAL
  []
  [L2p]
    type = ElementL2FunctorError
    approximate = pressure
    exact = zero
    execute_on = FINAL
  []
  [L2gradp]
    type = ElementVectorL2Error
    variable = reconstructed_grad_p
    function_x = 0
    function_y = 0
    execute_on = FINAL
  []
  [boundary_mass_balance]
    type = VolumetricFlowRate
    boundary = 'left right top bottom'
    vel_x = superficial_u
    vel_y = superficial_v
    advected_quantity = density
    rhie_chow_user_object = rc
    execute_on = FINAL
  []
  [p_max]
    type = ElementExtremeValue
    variable = pressure
    value_type = max
    execute_on = FINAL
  []
  [p_min]
    type = ElementExtremeValue
    variable = pressure
    value_type = min
    execute_on = FINAL
  []
  [pressure_error_range]
    type = ParsedPostprocessor
    expression = 'p_max-p_min'
    pp_names = 'p_max p_min'
    execute_on = FINAL
  []
[]

[Outputs]
  exodus = false
  [csv]
    type = CSV
    execute_on = FINAL
  []
[]
