##########################################################
# Conservation of mixture enthalpy in a closed cavity whose top wall is a boundary internal to the
# mesh: the interface with a block the energy equation is not defined on.
#
# The companion of energy-slip-wall-conservation.i, whose header describes the setup. There a wall
# is a mesh boundary; here the face has a neighbor element, but that element is outside the
# equation, so the face is still a wall and the enthalpy drift flux must carry nothing across it.
#
# The assertion is therefore analytic: the integral of rho_m cp_m T is invariant.
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
    ymax = 0.125
    nx = 8
    ny = 10
  []
  # Two rows of cells above the cavity form a second block the equation does not live on, so the
  # top of the cavity is a boundary internal to the mesh rather than a mesh boundary
  [solid]
    type = SubdomainBoundingBoxGenerator
    input = gen
    bottom_left = '0 0.1 0'
    top_right = '0.1 0.125 0'
    block_id = 1
  []
  [interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = solid
    primary_block = 0
    paired_block = 1
    new_boundary = interface
  []
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system energy_system'
  # Nothing is defined on the second block
  material_coverage_check = false
[]

[Physics]
  [NavierStokes]
    [FlowSegregated]
      [flow]
        block = 0
        compressibility = 'weakly-compressible'
        density = 'rho_mixture'
        dynamic_viscosity = 'mu_mixture'

        initial_velocity = '1e-12 1e-12 0'
        initial_pressure = 0

        wall_boundaries = 'interface left right bottom'
        momentum_wall_types = 'noslip noslip noslip noslip'
        momentum_wall_functors = '0 0; 0 0; 0 0; 0 0'

        orthogonality_correction = false
        pressure_two_term_bc_expansion = true
        momentum_advection_interpolation = 'upwind'
      []
    []
    [TwoPhaseMixtureSegregated]
      [mixture]
        block = 0
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
    block = 0
    type = MooseLinearVariableFVReal
    solver_sys = energy_system
    initial_condition = ${T_initial}
  []
[]

[AuxVariables]
  [phase_2]
    block = 0
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
    block = 0
    type = ParsedFunctorMaterial
    property_name = 'rho_cp'
    expression = 'rho_mixture * cp_mixture'
    functor_names = 'rho_mixture cp_mixture'
  []
  [enthalpy_density]
    block = 0
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
    block = 0
    type = LinearFVTimeDerivative
    variable = T_fluid
    factor = 'rho_cp'
  []
  [T_advection]
    block = 0
    type = LinearFVEnergyAdvection
    variable = T_fluid
    advected_quantity = 'temperature'
    cp = 'cp_mixture'
    advected_interp_method = upwind
    rhie_chow_user_object = ins_rhie_chow_interpolator
  []
  # The term under test. No 'slip_boundaries' is given, so every boundary is impermeable.
  [T_drift_flux]
    block = 0
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
    boundary = 'interface'
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
    block = 0
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
    block = 0
    type = ElementExtremeValue
    variable = T_fluid
  []
  [min_T]
    block = 0
    type = ElementExtremeValue
    variable = T_fluid
    value_type = min
  []
[]

[Outputs]
  csv = true
[]
