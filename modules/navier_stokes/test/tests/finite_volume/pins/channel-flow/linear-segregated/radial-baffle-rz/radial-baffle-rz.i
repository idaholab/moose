# Steady inviscid radial outflow through a porous annulus (r < r0) into a region of unit porosity
# (r > r0), separated by a pressure baffle on the cylinder r = r0, in RZ coordinates with x as the
# radial direction. With mass flow per radian m and the friction
# f = mu D U + 0.5 rho F |U| / eps U of LinearFVMomentumFriction, the exact solution is
#   U_r = m / (rho r), U_z = 0,
#   p = -m^2 / (2 rho eps^2 r^2) - mu D m / (rho eps) ln(r) + F m^2 / (2 rho eps^2 r) + c
# in each region. At the baffle the pressure jumps by the Bernoulli and form-loss terms of
# BernoulliFormLossPressureJump, with the porous side as the lower-porosity reference. The flow is
# inviscid because baffle faces carry no normal viscous stress, which a viscous solution of this
# form would need.

rho = 1
m = 1
eps_bed = 0.4
# Darcy coefficients mu D of each region. Central advection of a divergence-free flow adds almost
# nothing to the momentum diagonal, so without viscosity the friction keeps the momentum systems
# nonsingular, and the plenum needs it too.
darcy_bed = 1
darcy_plenum = 0.5
forchheimer = 2
form_loss = 0.5
r_inner = 1
r_baffle = 1.5
r_outer = 2
height = 0.25
nx = 8
ny = 2

[Problem]
  linear_sys_names = 'u_system v_system pressure_system'
  previous_nl_solution_required = true
[]

[Mesh]
  [generated]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = ${r_inner}
    xmax = ${r_outer}
    ymax = ${height}
    nx = ${nx}
    ny = ${ny}
  []
  [bed]
    type = ParsedSubdomainMeshGenerator
    input = generated
    combinatorial_geometry = 'x < ${r_baffle}'
    block_id = 1
    block_name = bed
  []
  [plenum]
    type = ParsedSubdomainMeshGenerator
    input = bed
    combinatorial_geometry = 'x > ${r_baffle}'
    block_id = 2
    block_name = plenum
  []
  [baffle]
    type = SideSetsBetweenSubdomainsGenerator
    input = plenum
    primary_block = bed
    paired_block = plenum
    new_boundary = baffle
  []
  coord_type = RZ
  rz_coord_axis = Y
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
    density = ${rho}
    form_loss = ${form_loss}
  []
  [rc]
    type = PorousRhieChowMassFlux
    u = superficial_u
    v = superficial_v
    pressure = pressure
    rho = ${rho}
    porosity = porosity
    p_diffusion_kernel = p_diffusion
    pressure_projection_method = consistent
    pressure_jump_models = pressure_jump
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
  [u_advection_stress]
    type = LinearPWCNSFVMomentumFlux
    variable = superficial_u
    advected_interp_method_name = average
    mu = 0
    u = superficial_u
    v = superficial_v
    momentum_component = x
    rhie_chow_user_object = rc
    use_nonorthogonal_correction = false
  []
  [v_advection_stress]
    type = LinearPWCNSFVMomentumFlux
    variable = superficial_v
    advected_interp_method_name = average
    mu = 0
    u = superficial_u
    v = superficial_v
    momentum_component = y
    rhie_chow_user_object = rc
    use_nonorthogonal_correction = false
  []
  [u_pressure]
    type = LinearPWCNSFVMomentumPressure
    variable = superficial_u
    pressure = pressure
    porosity = porosity
    momentum_component = x
    gradient_method = reconstructed
  []
  [v_pressure]
    type = LinearPWCNSFVMomentumPressure
    variable = superficial_v
    pressure = pressure
    porosity = porosity
    momentum_component = y
    gradient_method = reconstructed
  []
  [u_friction]
    type = LinearFVMomentumFriction
    variable = superficial_u
    Darcy_name = darcy
    Forchheimer_name = forchheimer
    mu = 1
    rho = ${rho}
    porosity = porosity
    u = superficial_u
    v = superficial_v
    momentum_component = x
  []
  [v_friction]
    type = LinearFVMomentumFriction
    variable = superficial_v
    Darcy_name = darcy
    Forchheimer_name = forchheimer
    mu = 1
    rho = ${rho}
    porosity = porosity
    u = superficial_u
    v = superficial_v
    momentum_component = y
  []
  [p_diffusion]
    type = LinearFVPressureCorrectionDiffusionJump
    variable = pressure
    rhie_chow_user_object = rc
    use_nonorthogonal_correction = false
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
    functor = 0
  []
  [pressure_flux]
    type = LinearFVPressureFluxBC
    boundary = 'left right top bottom'
    variable = pressure
    HbyA_flux = HbyA
    Ainv = Ainv
    rho = ${rho}
    u = superficial_u
    v = superficial_v
  []
[]

[FunctorMaterials]
  [porosity]
    type = PiecewiseByBlockFunctorMaterial
    prop_name = porosity
    subdomain_to_prop_value = 'bed ${eps_bed} plenum 1'
  []
  [friction_bed]
    type = GenericVectorFunctorMaterial
    prop_names = 'darcy forchheimer'
    prop_values = '${darcy_bed} ${darcy_bed} ${darcy_bed} ${forchheimer} ${forchheimer} ${forchheimer}'
    block = bed
  []
  [friction_plenum]
    type = GenericVectorFunctorMaterial
    prop_names = 'darcy forchheimer'
    prop_values = '${darcy_plenum} ${darcy_plenum} ${darcy_plenum} 0 0 0'
    block = plenum
  []
  [pressure_difference]
    type = ParsedFunctorMaterial
    property_name = pressure_difference
    functor_names = 'pressure exact_pressure'
    expression = 'pressure-exact_pressure'
  []
  # The pin fixes the pressure of the cell containing the pin point, so the computed pressure is
  # compared after removing the mean difference to the exact pressure.
  [shifted_pressure]
    type = ParsedFunctorMaterial
    property_name = shifted_pressure
    functor_names = 'pressure mean_pressure_difference'
    expression = 'pressure-mean_pressure_difference'
  []
  [pressure_error]
    type = ParsedFunctorMaterial
    property_name = pressure_error
    functor_names = 'shifted_pressure exact_pressure'
    expression = 'shifted_pressure-exact_pressure'
  []
  [u_error]
    type = ParsedFunctorMaterial
    property_name = u_error
    functor_names = 'superficial_u exact_u'
    expression = 'superficial_u-exact_u'
  []
[]

[Functions]
  [exact_u]
    type = ParsedFunction
    expression = '${m}/(${rho}*x)'
  []
  [bed_pressure]
    type = ParsedFunction
    expression = '-${m}^2/(2*${rho}*${eps_bed}^2*x^2) - ${darcy_bed}*${m}/(${rho}*${eps_bed})*log(x) '
                 '+ ${forchheimer}*${m}^2/(2*${rho}*${eps_bed}^2*x)'
  []
  [plenum_pressure]
    type = ParsedFunction
    expression = '-${m}^2/(2*${rho}*x^2) - ${darcy_plenum}*${m}/${rho}*log(x)'
  []
  # p_plenum(r0) - p_bed(r0) = 0.5 rho u0^2 (1/eps_bed^2 - 1) - 0.5 K rho (u0/eps_bed)^2 with
  # u0 = m / (rho r0).
  [exact_pressure]
    type = ParsedFunction
    symbol_names = 'pb pp u0'
    symbol_values = 'bed_pressure plenum_pressure ${fparse m / (rho * r_baffle)}'
    expression = 'if(x < ${r_baffle}, pb, pp + '
                 '(-${m}^2/(2*${rho}*${eps_bed}^2*${r_baffle}^2) - ${darcy_bed}*${m}/(${rho}*${eps_bed})*log(${r_baffle}) '
                 '+ ${forchheimer}*${m}^2/(2*${rho}*${eps_bed}^2*${r_baffle}) + ${m}^2/(2*${rho}*${r_baffle}^2) '
                 '+ ${darcy_plenum}*${m}/${rho}*log(${r_baffle})) '
                 '+ 0.5*${rho}*u0^2*(1/${eps_bed}^2 - 1) - 0.5*${form_loss}*${rho}*(u0/${eps_bed})^2)'
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
  # Without viscosity nothing damps round-off in the axial velocity, and at a momentum relaxation
  # of 0.5 the iteration amplifies it by about 1% per iteration on the twice-refined mesh.
  momentum_equation_relaxation = 0.3
  pressure_variable_relaxation = 0.2
  num_iterations = 3000
  pressure_absolute_tolerance = 1e-9
  # The exact axial velocity is zero, so the axial momentum system has a vanishing normalization
  # factor and its normalized residual stays O(1e-2) at round-off. Its tolerance is therefore not
  # used as a convergence criterion; L2v checks the axial velocity instead.
  momentum_absolute_tolerance = '1e-9 1'
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  print_fields = false

  pin_pressure = true
  pressure_pin_value = 0
  pressure_pin_point = '${fparse 0.5 * (r_baffle + r_outer)} ${fparse 0.5 * height} 0'
[]

[Outputs]
  [csv]
    type = CSV
    execute_on = FINAL
  []
[]

[Postprocessors]
  [mean_pressure_difference]
    type = ElementAverageFunctorPostprocessor
    functor = pressure_difference
    execute_on = FINAL
    # Must run before the pressure errors that subtract it.
    execution_order_group = -1
    outputs = none
  []
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
    exact = 0
    execute_on = FINAL
  []
  [L2p]
    type = ElementL2FunctorError
    approximate = shifted_pressure
    exact = exact_pressure
    execute_on = FINAL
  []
  # The maximum pointwise errors detect cell-to-cell oscillations that the L2 norms average out.
  [max_u_error]
    type = ElementExtremeFunctorValue
    functor = u_error
    value_type = max_abs
    execute_on = FINAL
  []
  [max_v_error]
    type = ElementExtremeFunctorValue
    functor = superficial_v
    value_type = max_abs
    execute_on = FINAL
  []
  [max_p_error]
    type = ElementExtremeFunctorValue
    functor = pressure_error
    value_type = max_abs
    execute_on = FINAL
  []
[]
