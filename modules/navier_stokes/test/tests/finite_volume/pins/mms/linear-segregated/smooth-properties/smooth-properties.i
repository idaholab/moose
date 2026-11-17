# Manufactured solution with continuously varying density and viscosity, optionally crossed by a
# pressure baffle with a porosity jump. In coordinates normal (xn) and tangent (xt) to the line
# x - skew * y = 0.5:
#  - rho = 1 + xn/2 + xt/4 + xn xt/5 and mu = mu0 (m0 + 4 xn^2)(1 + xt^2) are smooth. With m0 = 0.05
#    the viscosity varies by a factor of 40 across the domain and by a factor of several between
#    neighboring cells on the coarse meshes.
#  - rho U = curl(psi), psi = xt/2 + xt^2/10 + eps (xn (xt/10 - 3/10) + xn^2/5), so the mass flux is
#    divergence-free. U.n is continuous across xn = 0 and U_t jumps with eps, while the interstitial
#    tangential velocity U_t / eps is continuous.
#  - The pressure is smooth on each side. With a baffle, the right side adds the
#    BernoulliFormLossPressureJump evaluated with the interface density and normal velocity.
# The forcing contains the porous advection div(rho U U / eps), the full stress
# div(mu (grad U + grad U^T - 2/3 div U I)) of superficial velocity, eps grad p, and a reaction k U.
# Without a baffle (baffle = 0), eps_left = eps_right must hold.
# The expressions were generated with sympy.

eps_left = 1
eps_right = 1
form_loss = 0
mu0 = 0.01
m0 = 0.05
reaction = 1
skew = 0
nx = 8
ny = 8
# Set baffle = 1 together with UserObjects/rc/pressure_jump_models=pressure_jump to make the
# interface a pressure baffle; otherwise it is an ordinary internal face.
baffle = 0
deviatoric = true

# Porosity of the side whose interstitial velocity is the form-loss reference: the lower one, the
# BernoulliFormLossPressureJump default.
eps_ref = '${fparse min(eps_left, eps_right)}'

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
    combinatorial_geometry = 'x > 0.5'
    block_id = 2
    block_name = right
  []
  [left]
    type = ParsedSubdomainMeshGenerator
    input = right
    combinatorial_geometry = 'x < 0.5'
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
  # The reconstructed pressure gradient keeps the cell pressure gradient consistent with the face
  # fluxes and suppresses odd-even pressure modes.
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
    form_loss = ${form_loss}
  []
  [rc]
    type = PorousRhieChowMassFlux
    u = superficial_u
    v = superficial_v
    pressure = pressure
    rho = density
    porosity = porosity
    p_diffusion_kernel = p_diffusion
    pressure_projection_method = consistent
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
    mu = viscosity
    u = superficial_u
    v = superficial_v
    momentum_component = x
    rhie_chow_user_object = rc
    use_nonorthogonal_correction = true
    use_deviatoric_terms = ${deviatoric}
  []
  [v_advection_stress]
    type = LinearPWCNSFVMomentumFlux
    variable = superficial_v
    advected_interp_method_name = average
    mu = viscosity
    u = superficial_u
    v = superficial_v
    momentum_component = y
    rhie_chow_user_object = rc
    use_nonorthogonal_correction = true
    use_deviatoric_terms = ${deviatoric}
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
  [u_source]
    type = LinearFVSource
    variable = superficial_u
    source_density = forcing_u
  []
  [v_source]
    type = LinearFVSource
    variable = superficial_v
    source_density = forcing_v
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
    use_nonorthogonal_correction = true
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
    # The two-term boundary pressure expansion is inaccurate on this nonorthogonal mesh.
    use_two_term_expansion = false
  []
[]

[FunctorMaterials]
  [density]
    type = ParsedFunctorMaterial
    property_name = density
    functor_names = 'xn xt'
    expression = '1+xn/2+xt/4+xn*xt/5'
  []
  [viscosity]
    type = ParsedFunctorMaterial
    property_name = viscosity
    functor_names = 'xn xt'
    expression = '${mu0}*(${m0}+4*xn^2)*(1+xt^2)'
  []
  [porosity]
    type = ParsedFunctorMaterial
    property_name = porosity
    functor_names = 'exact_porosity'
    expression = 'exact_porosity'
  []
  [pressure_error]
    type = ParsedFunctorMaterial
    property_name = pressure_error
    functor_names = 'pressure exact_pressure'
    expression = 'pressure-exact_pressure'
  []
  [u_error]
    type = ParsedFunctorMaterial
    property_name = u_error
    functor_names = 'superficial_u exact_u'
    expression = 'superficial_u-exact_u'
  []
  [v_error]
    type = ParsedFunctorMaterial
    property_name = v_error
    functor_names = 'superficial_v exact_v'
    expression = 'superficial_v-exact_v'
  []
[]

[Functions]
  [xn]
    type = ParsedFunction
    expression = '(x-${skew}*y-0.5)/sqrt(1+${skew}^2)'
  []
  [xt]
    type = ParsedFunction
    expression = '(${skew}*x+y)/sqrt(1+${skew}^2)'
  []
  # Side indicator: 1 on the right of the interface when it is a baffle, otherwise 0.
  [rs]
    type = ParsedFunction
    expression = 'if(x-${skew}*y>0.5,${baffle},0)'
  []
  [exact_porosity]
    type = ParsedFunction
    expression = 'if(x-${skew}*y<0.5,${eps_left},${eps_right})'
  []
  [exact_normal_velocity]
    type = ParsedFunction
    symbol_names = 'xn xt eps'
    symbol_values = 'xn xt exact_porosity'
    expression = '2*(eps*xn + 2*xt + 5)/(4*xn*xt + 10*xn + 5*xt + 20)'
  []
  [exact_tangential_velocity]
    type = ParsedFunction
    symbol_names = 'xn xt eps'
    symbol_values = 'xn xt exact_porosity'
    expression = '2*eps*(-4*xn - xt + 3)/(4*xn*xt + 10*xn + 5*xt + 20)'
  []
  [exact_pressure]
    type = ParsedFunction
    symbol_names = 'xn xt rs el er eref K'
    symbol_values = 'xn xt rs ${eps_left} ${eps_right} ${eps_ref} ${form_loss}'
    expression = '(el^2*er^2*eref^2*(xt + 4)*((4*xn + 1)^2 + 2*(4*xn + 1)*(2*xt - 1) + 4*(2*xt - 1)^2)/160 - '
                 'rs*(2*xt + 5)^2*(K*el^2*er^2 + eref^2*(el^2 - er^2))/50)/(el^2*er^2*eref^2*(xt + 4))'
  []
  [forcing_normal]
    type = ParsedFunction
    symbol_names = 'xn xt eps mu0 m0 k'
    symbol_values = 'xn xt exact_porosity ${mu0} ${m0} ${reaction}'
    expression = '(3*eps^2*(2*xn + xt)*(4*xn*xt + 10*xn + 5*xt + 20)^3 + 60*eps*k*(eps*xn + 2*xt + '
                 '5)*(4*xn*xt + 10*xn + 5*xt + 20)^2 + 320*eps*mu0*xn*(xt^2 + 1)*(eps*(4*xn + 5)*(4*xn + xt '
                 '- 3) - 2*(2*xt + 5)*(eps*xn + 2*xt + 5))*(4*xn*xt + 10*xn + 5*xt + 20) + 40*eps*mu0*(m0 + '
                 '4*xn^2)*(xt^2 + 1)*(2*eps*(16*xn - 1)*(4*xn*xt + 10*xn + 5*xt + 20) - 3*(4*xn + '
                 '5)*(-2*eps*(2*xt + 5)*(4*xn + xt - 3) + (4*xn + 5)*(eps*xn + 2*xt + 5)) + 12*(2*xt + '
                 '5)*(eps*(4*xn*xt + 10*xn + 5*xt + 20) - 2*(2*xt + 5)*(eps*xn + 2*xt + 5)) + 4*(2*xt + '
                 '5)*(-eps*(4*xn + 5)*(4*xn + xt - 3) + 2*(2*xt + 5)*(eps*xn + 2*xt + 5)) - 3*(4*xn*xt + '
                 '10*xn + 5*xt + 20)*(2*eps*(4*xn + 5) + eps*(2*xt + 5) + 2*eps*(4*xn + xt - 3) - 8*xn - '
                 '10)) - 120*eps*mu0*(8*xn*(xt^2 + 1)*(eps*(4*xn*xt + 10*xn + 5*xt + 20) - 2*(2*xt + '
                 '5)*(eps*xn + 2*xt + 5)) + xt*(m0 + 4*xn^2)*(2*eps*(2*xt + 5)*(4*xn + xt - 3) + 2*(1 - '
                 '2*eps)*(4*xn*xt + 10*xn + 5*xt + 20) - (4*xn + 5)*(eps*xn + 2*xt + 5)))*(4*xn*xt + 10*xn + '
                 '5*xt + 20) + 6*(eps*(eps*xn - 8*xn + 11)*(4*xn*xt + 10*xn + 5*xt + 20) + (eps*(4*xn + '
                 '5)*(4*xn + xt - 3) - 2*(2*xt + 5)*(eps*xn + 2*xt + 5))*(eps*xn + 2*xt + 5))*(4*xn*xt + '
                 '10*xn + 5*xt + 20))/(30*eps*(4*xn*xt + 10*xn + 5*xt + 20)^3)'
  []
  [forcing_tangential]
    type = ParsedFunction
    symbol_names = 'xn xt eps rs el er eref K mu0 m0 k'
    symbol_values = 'xn xt exact_porosity rs ${eps_left} ${eps_right} ${eps_ref} ${form_loss} ${mu0} ${m0} ${reaction}'
    expression = '(-1200*el^2*eps*er^2*eref^2*k*(xt + 4)^2*(4*xn + xt - 3)*(4*xn*xt + 10*xn + 5*xt + 20)^2 + '
                 '1600*el^2*er^2*eref^2*mu0*xt*(m0 + 4*xn^2)*(xt + 4)^2*(eps*(4*xn + 5)*(4*xn + xt - 3) - '
                 '2*(2*xt + 5)*(eps*xn + 2*xt + 5))*(4*xn*xt + 10*xn + 5*xt + 20) + '
                 '400*el^2*er^2*eref^2*mu0*(m0 + 4*xn^2)*(xt + 4)^2*(xt^2 + 1)*(12*eps*(4*xn + 5)*(-4*xn*xt '
                 '- 10*xn - 5*xt + (4*xn + 5)*(4*xn + xt - 3) - 20) - 4*(4*xn + 5)*(eps*(4*xn + 5)*(4*xn + '
                 'xt - 3) - 2*(2*xt + 5)*(eps*xn + 2*xt + 5)) + 12*(2*xt + 5)*(2*eps*(2*xt + 5)*(4*xn + xt - '
                 '3) - (4*xn + 5)*(eps*xn + 2*xt + 5)) - 2*(4*eps*xn - eps*(4*xn + 5) + 16*xt + 40)*(4*xn*xt '
                 '+ 10*xn + 5*xt + 20) + 3*(4*xn*xt + 10*xn + 5*xt + 20)*(4*eps*xn + eps*(4*xn + 5) - '
                 '16*eps*(2*xt + 5) + 16*xt + 40)) - 4800*el^2*er^2*eref^2*mu0*(xt + 4)^2*(eps*xt*(m0 + '
                 '4*xn^2)*(-4*xn*xt - 10*xn - 5*xt + (4*xn + 5)*(4*xn + xt - 3) - 20) + 2*xn*(xt^2 + '
                 '1)*(2*eps*(2*xt + 5)*(4*xn + xt - 3) + 2*(1 - 2*eps)*(4*xn*xt + 10*xn + 5*xt + 20) - (4*xn '
                 '+ 5)*(eps*xn + 2*xt + 5)))*(4*xn*xt + 10*xn + 5*xt + 20) + 120*el^2*er^2*eref^2*(xt + '
                 '4)^2*(-(eps*(4*xn + 5)*(4*xn + xt - 3) - 2*(2*xt + 5)*(eps*xn + 2*xt + 5))*(4*xn + xt - 3) '
                 '+ (-4*eps*xn + eps*(4*xn + xt - 3) - 8*xt - 20)*(4*xn*xt + 10*xn + 5*xt + 20))*(4*xn*xt + '
                 '10*xn + 5*xt + 20) - 3*eps*(5*el^2*er^2*eref^2*(xt + 4)^2*(-4*xn - 8*xt + 3) + 4*rs*(2*xt '
                 '+ 5)*(4*K*el^2*er^2*(xt + 4) - K*el^2*er^2*(2*xt + 5) + eref^2*(-el^2 + er^2)*(2*xt + 5) + '
                 '4*eref^2*(el^2 - er^2)*(xt + 4)))*(4*xn*xt + 10*xn + 5*xt + '
                 '20)^3)/(600*el^2*er^2*eref^2*(xt + 4)^2*(4*xn*xt + 10*xn + 5*xt + 20)^3)'
  []
  [exact_u]
    type = ParsedFunction
    expression = '(un+${skew}*ut)/sqrt(1+${skew}^2)'
    symbol_names = 'un ut'
    symbol_values = 'exact_normal_velocity exact_tangential_velocity'
  []
  [exact_v]
    type = ParsedFunction
    expression = '(-${skew}*un+ut)/sqrt(1+${skew}^2)'
    symbol_names = 'un ut'
    symbol_values = 'exact_normal_velocity exact_tangential_velocity'
  []
  [forcing_u]
    type = ParsedFunction
    expression = '(fn+${skew}*ft)/sqrt(1+${skew}^2)'
    symbol_names = 'fn ft'
    symbol_values = 'forcing_normal forcing_tangential'
  []
  [forcing_v]
    type = ParsedFunction
    expression = '(-${skew}*fn+ft)/sqrt(1+${skew}^2)'
    symbol_names = 'fn ft'
    symbol_values = 'forcing_normal forcing_tangential'
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
  momentum_equation_relaxation = 0.5
  pressure_variable_relaxation = 0.2
  num_iterations = 3000
  pressure_absolute_tolerance = 1e-9
  momentum_absolute_tolerance = 1e-9
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  print_fields = false

  # The exact pressure is zero at (xn, xt) = (-1/4, 1/2), on the left side.
  pin_pressure = true
  pressure_pin_value = 0
  pressure_pin_point = '${fparse 0.5 + (-0.25 + skew * (0.5 - 0.5 * skew / sqrt(1 + skew^2))) / sqrt(1 + skew^2)} '
                     '${fparse (0.25 * skew + 0.5 - 0.5 * skew / sqrt(1 + skew^2)) / sqrt(1 + skew^2)} 0'
[]

[Outputs]
  [csv]
    type = CSV
    execute_on = FINAL
  []
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
    functor = v_error
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
