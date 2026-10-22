# Fluid at rest under gravity in a closed box split by a pressure baffle across which porosity
# (and optionally density) jumps. The interface x - skew * y = interface is a mesh line.
# The superficial momentum balance at rest is 0 = -eps grad(p) + eps rho g, so
#   U = 0 and grad(p) = rho g on both sides,
# independent of porosity. With density rho = rho_s + strat * z linear in the height
# z = -g . x / |g|, the exact pressure is
#   p = -|g| (rho_s z + strat z^2 / 2) + const.
# A density jump (rho_left != rho_right) is hydrostatic only if the interface is a level
# surface (gravity along the interface normal). With mdot = 0 the Bernoulli jump vanishes.
# The body force is the porosity-weighted buoyancy eps rho g, applied either by
# LinearFVMomentumBuoyancy or, with LinearFVKernels/inactive = 'u_buoyancy v_buoyancy', by
# LinearFVMomentumBoussinesq with a temperature chosen so that -alpha rho_ref (T - T_ref) = rho.

rho_left = 1
rho_right = 1
strat = -0.1
eps_left = 1
eps_right = 1
mu = 0.01
reaction = 1
gx = 0
gy = -9.81
interface = 0.5
skew = 0
nx = 8
ny = 8
nonorthogonal = false
alpha = 1e-3
T_ref = 300
pressure_gradient_method = reconstructed

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
  [velocity_jump_aware]
    type = FVVelocityJumpGreenGaussGradient
  []
  [reconstructed]
    type = FVReconstructedPressureGradient
    base_gradient_method = jump_aware
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
    # The body force enters H/A through face averages, so the 1/A diffusion must be averaged the
    # same way for the hydrostatic balance to hold on the face when the viscous A varies.
    pressure_diffusion_interpolation = average
  []
[]

[Variables]
  [superficial_u]
    type = MooseLinearVariableFVReal
    solver_sys = u_system
    initial_condition = 0
    gradient_method = velocity_jump_aware
  []
  [superficial_v]
    type = MooseLinearVariableFVReal
    solver_sys = v_system
    initial_condition = 0
    gradient_method = velocity_jump_aware
  []
  [pressure]
    type = MooseLinearVariableFVReal
    solver_sys = pressure_system
    initial_condition = 0
    gradient_method = jump_aware
  []
[]

[LinearFVKernels]
  inactive = 'u_boussinesq v_boussinesq'
  [u_advection]
    type = LinearPWCNSFVMomentumFlux
    variable = superficial_u
    advected_interp_method_name = average
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
  [u_buoyancy]
    type = LinearFVMomentumBuoyancy
    variable = superficial_u
    rho = density
    porosity = porosity
    reference_rho = 0
    gravity = '${gx} ${gy} 0'
    momentum_component = x
  []
  [v_buoyancy]
    type = LinearFVMomentumBuoyancy
    variable = superficial_v
    rho = density
    porosity = porosity
    reference_rho = 0
    gravity = '${gx} ${gy} 0'
    momentum_component = y
  []
  [u_boussinesq]
    type = LinearFVMomentumBoussinesq
    variable = superficial_u
    T_fluid = T_fluid
    rho = 1
    alpha_name = ${alpha}
    ref_temperature = ${T_ref}
    porosity = porosity
    gravity = '${gx} ${gy} 0'
    momentum_component = x
  []
  [v_boussinesq]
    type = LinearFVMomentumBoussinesq
    variable = superficial_v
    T_fluid = T_fluid
    rho = 1
    alpha_name = ${alpha}
    ref_temperature = ${T_ref}
    porosity = porosity
    gravity = '${gx} ${gy} 0'
    momentum_component = y
  []
  [u_reaction]
    type = LinearFVReaction
    variable = superficial_u
    coeff = ${reaction}
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
    functor = 0
  []
  [velocity_y]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left right top bottom'
    variable = superficial_v
    functor = 0
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
    functor_names = 'exact_density'
    expression = 'exact_density'
  []
  [porosity]
    type = ParsedFunctorMaterial
    property_name = porosity
    expression = 'if(x-${skew}*y<${interface},${eps_left},${eps_right})'
  []
  [pressure_error]
    type = ParsedFunctorMaterial
    property_name = pressure_error
    functor_names = 'pressure exact_pressure'
    expression = 'pressure-exact_pressure'
  []
  [speed]
    type = ParsedFunctorMaterial
    property_name = speed
    functor_names = 'superficial_u superficial_v'
    expression = 'sqrt(superficial_u^2+superficial_v^2)'
  []
[]

[AuxVariables]
  [T_fluid]
    type = MooseLinearVariableFVReal
  []
[]

[AuxKernels]
  [T_fluid]
    type = FunctorAux
    variable = T_fluid
    functor = boussinesq_temperature
    execute_on = INITIAL
  []
[]

[Functions]
  [boussinesq_temperature]
    type = ParsedFunction
    expression = 'T_ref-rho/alpha'
    symbol_names = 'T_ref rho alpha'
    symbol_values = '${T_ref} exact_density ${alpha}'
  []
  [height]
    type = ParsedFunction
    expression = '-(gx*x+gy*y)/sqrt(gx^2+gy^2)'
    symbol_names = 'gx gy'
    symbol_values = '${gx} ${gy}'
  []
  [exact_density]
    type = ParsedFunction
    expression = 'if(x-skew*y<interface,rho_left,rho_right)+strat*hz'
    symbol_names = 'skew interface rho_left rho_right strat hz'
    symbol_values = '${skew} ${interface} ${rho_left} ${rho_right} ${strat} height'
  []
  # Hydrostatic pressure with the pin at (0.3, 0.3), inside a cell on 2^n meshes, set to zero. The pin is on the left side,
  # and a density jump is allowed only across a level interface, where the side constants
  # rho_s z_interface agree.
  [raw_pressure]
    type = ParsedFunction
    expression = '-sqrt(gx^2+gy^2)*(if(x-skew*y<interface,rho_left,rho_right)*(hz-hi)+rho_left*hi+strat*hz^2/2)'
    symbol_names = 'gx gy skew interface rho_left rho_right strat hz hi'
    symbol_values = '${gx} ${gy} ${skew} ${interface} ${rho_left} ${rho_right} ${strat} height '
                    '${fparse -(gx*interface)/sqrt(gx^2+gy^2)}'
  []
  [exact_pressure]
    type = ParsedFunction
    expression = 'p+sqrt(gx^2+gy^2)*(rho_left*hp+strat*hp^2/2)'
    symbol_names = 'p gx gy rho_left strat hp'
    symbol_values = 'raw_pressure ${gx} ${gy} ${rho_left} ${strat} '
                    '${fparse -(gx*(0.3+skew*0.3)+gy*0.3)/sqrt(gx^2+gy^2)}'
  []
[]

[Executioner]
  type = SIMPLE
  momentum_l_abs_tol = 1e-13
  pressure_l_abs_tol = 1e-13
  momentum_l_tol = 0
  pressure_l_tol = 0
  momentum_l_max_its = 100
  pressure_l_max_its = 100
  rhie_chow_user_object = rc
  momentum_systems = 'u_system v_system'
  pressure_system = pressure_system
  momentum_equation_relaxation = 0.7
  pressure_variable_relaxation = 0.5
  num_iterations = 300
  continue_on_max_its = true
  pressure_absolute_tolerance = 1e-12
  momentum_absolute_tolerance = 1e-12
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  print_fields = false

  pin_pressure = true
  pressure_pin_value = 0
  pressure_pin_point = '${fparse 0.3 + skew * 0.3} 0.3 0'
[]

[Postprocessors]
  [max_speed]
    type = ElementExtremeFunctorValue
    functor = speed
    value_type = max
    execute_on = FINAL
  []
  # The pin sits at a point, not a cell centroid, so the error is compared up to a constant.
  [max_pressure_error]
    type = ElementExtremeFunctorValue
    functor = pressure_error
    value_type = max
    execute_on = FINAL
    outputs = none
  []
  [min_pressure_error]
    type = ElementExtremeFunctorValue
    functor = pressure_error
    value_type = min
    execute_on = FINAL
    outputs = none
  []
  [pressure_error_range]
    type = ParsedPostprocessor
    pp_names = 'max_pressure_error min_pressure_error'
    expression = 'max_pressure_error-min_pressure_error'
    execute_on = FINAL
  []
[]

[Outputs]
  [csv]
    type = CSV
    execute_on = FINAL
  []
[]
