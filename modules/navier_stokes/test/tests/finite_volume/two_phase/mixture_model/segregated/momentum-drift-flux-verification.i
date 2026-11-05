# Verification of the diffusion stress assembled by LinearWCNSFV2PMomentumDriftFlux against a
# solution worked out by hand.
#
# The kernel adds div(beta_d beta_c / rho_m u_s u_s) to the left hand side of the equation it is
# attached to. Here that equation is a one dimensional Poisson problem for a scalar w, with the
# slip velocity a prescribed unit constant along x and the two phase densities equal to one, so
# that beta_d = alpha, beta_c = 1 - alpha and rho_m = 1:
#
#   -w'' + (alpha (1 - alpha))' = 0,   w(0) = w(1) = 0,   alpha = x
#
#   w = x^2/2 - x^3/3 - x/6
#
# The three defects this guards against each give a different w. With the sign of the stress
# reversed, w changes sign. With the dilute-limit coefficient alpha rho_d in place of the exact
# one, the forcing becomes (alpha)' and w = x^2/2 - x/2. Neither is within the discretisation
# error of the exact solution on this mesh, which is a few parts in ten thousand.
#
# The kernel assembles its term as a deferred correction, so the fixed point iteration that
# converges it is supplied by stepping a transient with no time derivative: each step re-solves
# the same steady problem with the previous solution as the lagged iterate. The surrogate is
# bounded by the slip flux, which is small against the diffusion here, so a handful of steps is
# enough for the correction to settle to round-off.

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 0
    xmax = 1
    nx = 40
  []
[]

[Problem]
  linear_sys_names = 'w_system'
  # The deferred correction reads the previous iterate
  previous_nl_solution_required = true
[]

[Variables]
  [w]
    type = MooseLinearVariableFVReal
    solver_sys = w_system
  []
[]

[Functions]
  [w_exact]
    type = ParsedFunction
    expression = 'x^2/2 - x^3/3 - x/6'
  []
  [alpha]
    type = ParsedFunction
    expression = 'x'
  []
[]

[LinearFVKernels]
  [diffusion]
    type = LinearFVDiffusion
    variable = w
    diffusion_coeff = 1.0
    use_nonorthogonal_correction = false
  []
  [drift_stress]
    type = LinearWCNSFV2PMomentumDriftFlux
    variable = w
    momentum_component = x
    u_slip = 1.0
    rho_d = 1.0
    rho_c = 1.0
    fraction_dispersed = alpha
    # The coefficient is a weight on an advective flux, not a diffusivity, so the arithmetic
    # average is the second order choice for it
    density_interp_method = average
  []
[]

[LinearFVBCs]
  [ends]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = w
    boundary = 'left right'
    functor = 0
  []
[]

[Postprocessors]
  [l2_error]
    type = ElementL2FunctorError
    approximate = w
    exact = w_exact
    execute_on = 'TIMESTEP_END'
  []
  # The mean of the exact solution vanishes, so the mean alone cannot tell a sign error from the
  # right answer; the first moment can, and is 1/360.
  [first_moment]
    type = ElementIntegralFunctorPostprocessor
    functor = w_times_x
    execute_on = 'TIMESTEP_END'
  []
[]

[FunctorMaterials]
  [moment]
    type = ParsedFunctorMaterial
    property_name = w_times_x
    expression = 'w * x'
    functor_names = 'w'
  []
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 8
  system_names = w_system
  l_tol = 1e-12
[]

[Outputs]
  csv = true
[]
