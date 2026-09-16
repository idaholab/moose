##########################################################
# Conservation of mixture enthalpy in a closed cavity carrying an enthalpy drift flux.
#
# The energy counterpart of phase-slip-wall-conservation.i, and the test of the boundary guard on
# LinearWCNSFV2PEnergyDriftFlux. The two phases are given different specific heats so that the
# enthalpy carried by their relative motion, which is proportional to (cp_d - cp_c), does not
# vanish, and the phase fraction is stratified so that the coefficient of the term varies in space.
# The slip velocity is a prescribed uniform constant normal to the top and bottom walls.
#
# Integrating the energy equation over a closed domain leaves only boundary fluxes. The mixture
# advection carries none, because the mixture velocity is zero at a no-slip wall, and there is no
# conduction kernel, so conduction carries none either. The enthalpy drift flux is the only term
# that could carry enthalpy through a wall, and it must not: no phase crosses an impermeable wall,
# so no enthalpy may be carried through one by the relative motion of the phases. The assertion is
# therefore analytic: the integral of rho_m cp_m T is invariant.
#
# The phase fraction is held fixed as an auxiliary field rather than transported. That is what makes
# the assertion exact rather than approximate: with a transported phase the factor rho_m cp_m in
# the time derivative moves between outer iterations, and the discrete balance only closes once the
# segregated coupling has converged. Holding it fixed leaves a wall leak as the only mechanism able
# to move the integral at all.
#
# The temperature is pinned on the top and bottom walls. Without a boundary condition there the
# flux kernels contribute nothing on those faces and the guard would be untestable.
##########################################################

mu = 1.0
rho = 1e3
mu_d = 0.3
rho_d = 1.0
dp = 0.01
slip = 0.05
T_initial = 300

# Currently required by the mixture Physics
k = 1
k_d = 1
cp = 1
# Different from cp, so that the enthalpy drift flux does not vanish identically
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
  linear_sys_names = 'u_system v_system pressure_system energy_system'
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
        system_names = 'energy_system'
        phase_1_fraction_name = 'phase_1'
        phase_2_fraction_name = 'phase_2'

        # The phase fraction is a fixed auxiliary field, see the header
        add_phase_transport_equation = false

        phase_1_density_name = ${rho}
        phase_1_viscosity_name = ${mu}
        phase_1_specific_heat_name = ${cp}
        phase_1_thermal_conductivity_name = ${k}

        phase_2_density_name = ${rho_d}
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
  [T_fluid]
    type = MooseLinearVariableFVReal
    solver_sys = energy_system
    initial_condition = ${T_initial}
  []
[]

[AuxVariables]
  [phase_2]
    type = MooseLinearVariableFVReal
    [AuxKernel]
      type = FunctionAux
      function = stratified
      execute_on = 'INITIAL'
    []
  []
[]

[Functions]
  [stratified]
    # Non-uniform, so that the coefficient of the enthalpy drift flux varies in space
    type = ParsedFunction
    expression = 'if(y > 0.05, 0.7, 0.3)'
  []
[]

[FunctorMaterials]
  # The factor in the energy time derivative, and the integrand of the conserved quantity. The
  # mixture specific heat created by the Physics is the mass-weighted average, so this product is
  # the mixture enthalpy density per unit temperature.
  [rho_cp]
    type = ParsedFunctorMaterial
    property_name = 'rho_cp'
    expression = 'rho_mixture * cp_mixture'
    functor_names = 'rho_mixture cp_mixture'
  []
  [enthalpy_density]
    type = ParsedFunctorMaterial
    property_name = 'enthalpy_density'
    expression = 'rho_cp * T_fluid'
    functor_names = 'rho_cp T_fluid'
  []
[]

# The energy equation, written out so that the slip velocity is a prescribed uniform constant and
# so that no conduction kernel is present. The only terms able to reach a wall are the two
# advective ones.
[LinearFVKernels]
  [T_time]
    type = LinearFVTimeDerivative
    variable = T_fluid
    factor = 'rho_cp'
  []
  [T_advection]
    type = LinearFVEnergyAdvection
    variable = T_fluid
    advected_quantity = 'temperature'
    cp = 'cp_mixture'
    advected_interp_method = upwind
    rhie_chow_user_object = ins_rhie_chow_interpolator
  []
  # The term under test. No 'slip_boundaries' is given, so every boundary is impermeable.
  [T_drift_flux]
    type = LinearWCNSFV2PEnergyDriftFlux
    variable = T_fluid
    advected_interp_method = upwind
    u_slip = 0
    v_slip = ${slip}
    rho_d = ${rho_d}
    rho_c = ${rho}
    cp_d = ${cp_d}
    cp_c = ${cp}
    fraction_dispersed = phase_2
  []
[]

[LinearFVBCs]
  [top_T]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T_fluid
    boundary = 'top'
    functor = ${T_initial}
  []
  [bottom_T]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T_fluid
    boundary = 'bottom'
    functor = ${T_initial}
  []
[]

[Executioner]
  type = PIMPLE
  rhie_chow_user_object = 'ins_rhie_chow_interpolator'

  num_steps = 4
  dt = 0.05

  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  energy_system = 'energy_system'
  momentum_equation_relaxation = 0.8
  energy_equation_relaxation = 0.9
  pressure_variable_relaxation = 0.3

  num_iterations = 50
  pressure_absolute_tolerance = 1e-11
  momentum_absolute_tolerance = 1e-11
  energy_absolute_tolerance = 1e-11
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  energy_petsc_options_iname = '-pc_type -pc_factor_shift_type'
  energy_petsc_options_value = 'lu NONZERO'
  momentum_l_abs_tol = 1e-13
  pressure_l_abs_tol = 1e-13
  energy_l_abs_tol = 1e-13
  continue_on_max_its = true

  pin_pressure = true
  pressure_pin_value = 0.0
  pressure_pin_point = '0.0 0.0 0.0'
[]

[Postprocessors]
  [enthalpy_integral]
    type = ElementIntegralFunctorPostprocessor
    functor = 'enthalpy_density'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  # The assertion: this must stay at zero
  [enthalpy_integral_drift]
    type = ChangeOverTimePostprocessor
    postprocessor = enthalpy_integral
    change_with_respect_to_initial = true
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [max_T]
    type = ElementExtremeValue
    variable = T_fluid
  []
  [min_T]
    type = ElementExtremeValue
    variable = T_fluid
    value_type = min
  []
[]

[Outputs]
  csv = true
[]
