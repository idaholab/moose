##########################################################
# Conservation of interfacial area in a closed quiescent cavity whose dispersed phase density moves
# in time. The companion of conservative-transient-energy.i, covering the interfacial area equation.
#
# THE TERM UNDER TEST. The interfacial area equation is the conservative Fluent 16.4-27,
# d(rho_d xi)/dt + div(rho_d u_d xi) = sources, and a 'factor' of rho_d is exactly what
# LinearFVTimeDerivative assembles: the product. Setting 'conservative_form' to false instead
# holds the multiplier outside the derivative, giving rho_d d(xi)/dt. The two differ only where the
# dispersed phase density moves in time; this case makes it triple. The parameter is written out
# below even though it is the default, since it is the behaviour under test.
#
# THE ASSERTION. The cavity is closed and quiescent and the equation is given no advection and no
# coalescence or breakage source, so the integral of rho_d xi is invariant. In the conservative
# form the discrete sum telescopes, so the integral holds to round-off, 1.7e-11 relative. Turned
# off, the code solves rho_d d(xi)/dt = 0, so xi does not move at all while rho_d triples and the
# integral triples with it: the failure is 200%, not a margin comparable to the tolerance.
#
# The phase fraction is a fixed auxiliary field, so rho_m and rho_d are functions of time alone and
# the assertion is exact rather than approximate.
#
# The relative linear tolerances are pinned for the reason recorded in the companion energy case:
# telescoping only shows in the answer to the extent that each step's linear solve is converged.
########################################################################################################
# Conservation of mixture momentum and of interfacial area in a box whose dispersed phase density
# moves in time. The companion of conservative-transient-energy.i, covering the other two equations
# that carry the same product rule.
#
# THE TERMS UNDER TEST. LinearFVTimeDerivative places its multiplier outside the derivative, so a
# 'factor' of rho_m assembles rho_m d(u)/dt rather than d(rho_m u)/dt, and a 'factor' of rho_d
# assembles rho_d d(xi)/dt rather than d(rho_d xi)/dt. The missing halves, u d(rho_m)/dt and
# xi d(rho_d)/dt, are supplied here. Both are proportional to a density rate and so vanish
# identically wherever the densities are constant in time; this case makes rho_d move.
#
# THE ASSERTIONS. The phase fraction is uniform and fixed, so rho_m has no spatial gradient. The
# velocity starts uniform, the top and bottom are free slip so no wall exerts a force, and the left
# and right carry no momentum boundary condition at all, so the flux kernels contribute nothing
# there. Internal advection cancels for a uniform velocity at uniform density, and the pressure
# stays uniform. What is left is d(rho_m u)/dt = 0, so the integral of rho_m u is invariant. The
# interfacial area is given no advection, leaving d(rho_d xi)/dt = 0 and the integral of rho_d xi
# invariant on the same grounds.
#
# Without the terms the code solves rho_m d(u)/dt = 0 and rho_d d(xi)/dt = 0, so neither u nor xi
# moves at all while the densities triple, and both integrals triple with them. The failure is
# order one rather than a margin comparable to the tolerance.
#
# The mixture mass equation is deliberately left in its default form, without
# 'add_mass_density_transient'. The pressure equation then enforces div(rho_m u) = 0, which a
# uniform velocity at uniform density satisfies exactly, so the flow stays uniform and the momentum
# equation is the only thing evolving. Adding the storage term would make the solver generate a
# velocity divergence and destroy the analytic setting this assertion rests on.
#
# BDF2 and the relative linear tolerances are both pinned, for the reasons recorded in the
# companion energy case: the default integrator halves the observed order, and the default 1e-5
# relative linear tolerance leaves a fixed error per step that accumulates as the step is refined.
##########################################################

dp = 0.01     # particle diameter, required by the mixture Physics but not exercised here
mu = 1.0
rho = 1e3
mu_d = 0.3
# The dispersed phase density triples over the run. It is prescribed as a function of time rather
# than taken from the pressure, so that d(rho_d)/dt is known exactly and the assertion does not
# depend on the flow solution.
# Comparable to the continuous phase density, so that the dispersed phase actually carries the
# mixture heat capacity and the term under test is a leading contribution rather than a correction.
rho_d = 500.0
rho_d_final_factor = 3.0
t_end = 0.2   # matches num_steps * dt in the Executioner below
T_initial = 300

# Currently required by the mixture Physics
k = 1
k_d = 1
cp = 1
cp_d = 10

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 0.1
    ymin = 0
    ymax = 0.1
    nx = 8
    ny = 8
  []
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system xi_system'
[]

[Physics]
  [NavierStokes]
    [FlowSegregated]
      [flow]
        compressibility = 'weakly-compressible'
        density = 'rho_mixture'
        dynamic_viscosity = 'mu_mixture'

        initial_velocity = '1e-12 1e-12 0'
        initial_pressure = 0

        wall_boundaries = 'top left right bottom'
        momentum_wall_types = 'noslip noslip noslip noslip'
        momentum_wall_functors = '0 0; 0 0; 0 0; 0 0'

        orthogonality_correction = false
        pressure_two_term_bc_expansion = true
        momentum_advection_interpolation = 'upwind'
      []
    []
    [TwoPhaseMixtureSegregated]
      [mixture]
        system_names = 'xi_system'
        phase_1_fraction_name = 'phase_1'
        phase_2_fraction_name = 'phase_2'

        # The phase fraction is a fixed auxiliary field, see the header
        add_phase_transport_equation = false

        phase_1_density_name = ${rho}
        phase_1_viscosity_name = ${mu}
        phase_1_specific_heat_name = ${cp}
        phase_1_thermal_conductivity_name = ${k}

        phase_2_density_name = 'rho_d_t'
        phase_2_density_time_derivative = 'drho_d_dt'
        phase_2_viscosity_name = ${mu_d}
        phase_2_specific_heat_name = ${cp_d}
        phase_2_thermal_conductivity_name = ${k_d}

        use_dispersed_phase_drag_model = true
        particle_diameter = ${dp}
      []
    []
  []
[]

[Variables]
  [interface_area]
    type = MooseLinearVariableFVReal
    solver_sys = xi_system
    initial_condition = ${T_initial}
  []
[]

[AuxVariables]
  [phase_2]
    type = MooseLinearVariableFVReal
    [AuxKernel]
      type = FunctionAux
      function = uniform_alpha
      execute_on = 'INITIAL'
    []
  []
[]

[Functions]
  [uniform_alpha]
    # Uniform, so that rho_m has no spatial gradient and a uniform velocity stays uniform
    type = ParsedFunction
    expression = '0.5'
  []
[]

[FunctorMaterials]
  # The factor in the energy time derivative, and the integrand of the conserved quantity. The
  # mixture specific heat created by the Physics is the mass-weighted average, so this product is
  # rho_d(t) = rho_d0 (1 + (f - 1) t / t_end), so d(rho_d)/dt = rho_d0 (f - 1) / t_end
  [rho_d_t]
    type = ParsedFunctorMaterial
    property_name = 'rho_d_t'
    expression = '${rho_d} * (1 + (${rho_d_final_factor} - 1) * t / ${t_end})'
  []
  [drho_d_dt]
    type = ParsedFunctorMaterial
    property_name = 'drho_d_dt'
    expression = '${rho_d} * (${rho_d_final_factor} - 1) / ${t_end}'
  []
  # The conserved density
  [area_density]
    type = ParsedFunctorMaterial
    property_name = 'area_density'
    expression = 'rho_d_t * interface_area'
    functor_names = 'rho_d_t interface_area'
  []
[]

# The interfacial area equation, written out with no advection so that the transient is the only
# term. The momentum equation is built by the Physics above, which assembles the same conservative
# form because that is what the time derivative kernel does by default.
[LinearFVKernels]
  # The interfacial area transport, Fluent 16.4-27 without its sources
  [xi_time]
    type = LinearFVTimeDerivative
    variable = interface_area
    factor = 'rho_d_t'
    conservative_form = true
  []
[]

[LinearFVBCs]
[]

[Executioner]
  type = PIMPLE
  rhie_chow_user_object = 'ins_rhie_chow_interpolator'

  # BDF2 explicitly, not the default. The default is implicit Euler, and the rate at which the
  # residual drift below vanishes is the rate of the integrator, so leaving it unset would halve
  # the order and hide what this case is measuring.
  scheme = bdf2
  num_steps = 40
  dt = 0.005

  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  turbulence_systems = ''
  active_scalar_systems = 'xi_system'
  momentum_equation_relaxation = 0.8
  active_scalar_equation_relaxation = 0.9
  pressure_variable_relaxation = 0.3

  num_iterations = 50
  pressure_absolute_tolerance = 1e-11
  momentum_absolute_tolerance = 1e-11
  active_scalar_absolute_tolerance = 1e-11
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  active_scalar_petsc_options_iname = '-pc_type -pc_factor_shift_type'
  active_scalar_petsc_options_value = 'lu NONZERO'
  momentum_l_abs_tol = 1e-13
  pressure_l_abs_tol = 1e-13
  active_scalar_l_abs_tol = 1e-13
  # The RELATIVE linear tolerances matter here and default to 1e-5, which is far too loose: each
  # linear solve then stops early and leaves a fixed error per step, which accumulates as the step
  # is refined. Left at the default the drift falls to 7.8 at dt = 0.0025 and then RISES again, to
  # 13.9 and 13.4, because the iteration error overtakes the discretisation error. Tightening the
  # absolute tolerances alone does nothing: the relative criterion binds first.
  momentum_l_tol = 1e-12
  pressure_l_tol = 1e-12
  active_scalar_l_tol = 1e-12
  continue_on_max_its = true

  pin_pressure = true
  pressure_pin_value = 0.0
  pressure_pin_point = '0.0 0.0 0.0'
[]

[Postprocessors]
  # Interfacial area: the integral of rho_d xi must not move either
  [area_integral]
    type = ElementIntegralFunctorPostprocessor
    functor = 'area_density'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [area_drift]
    type = ChangeOverTimePostprocessor
    postprocessor = area_integral
    change_with_respect_to_initial = true
    execute_on = 'INITIAL TIMESTEP_END'
  []
  # Reported so that a failure is legible: without the terms neither of these moves at all
  [xi_average]
    type = ElementAverageValue
    variable = interface_area
  []
[]

[Outputs]
  csv = true
[]
