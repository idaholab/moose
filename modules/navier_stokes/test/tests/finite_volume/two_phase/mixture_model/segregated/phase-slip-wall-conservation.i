##########################################################
# Conservation of the dispersed phase in a closed cavity.
#
# There are no inlets, no outlets, no phase fraction boundary conditions and no phase diffusion, so
# the only way the phase fraction integral can move is if the advection kernel transports phase
# across a wall. Gravity is on, so the slip velocity is non-zero and normal to the top and bottom
# walls, which is exactly the situation in which applying the slip on a boundary face would leak.
#
# The assertion is therefore analytic: the integral of the phase fraction is invariant.
##########################################################

mu = 1.0
rho = 1e3
mu_d = 0.3
rho_d = 1.0
dp = 0.01
g = 0
slip = 0.05

# Currently required by the mixture Physics
k = 1
k_d = 1
cp = 1
cp_d = 1

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
  linear_sys_names = 'u_system v_system pressure_system phi_system'
[]

[Physics]
  [NavierStokes]
    [FlowSegregated]
      [flow]
        compressibility = 'weakly-compressible'
        density = 'rho_mixture'
        dynamic_viscosity = 'mu_mixture'
        gravity = '0 ${g} 0'

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
        system_names = 'phi_system'
        phase_1_fraction_name = 'phase_1'
        phase_2_fraction_name = 'phase_2'

        # The phase equation is declared explicitly below instead
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
  [phase_2]
    type = MooseLinearVariableFVReal
    solver_sys = phi_system
    initial_condition = 0.5
  []
[]

[ICs]
  [stratify]
    type = FunctionIC
    variable = phase_2
    function = stratified
  []
[]

# The phase equation, written out so that the slip velocity is a prescribed uniform constant.
# That removes the feedback phase fraction -> mixture density -> slip -> phase fraction, which is
# what makes the closure driven version of this case stiff, and leaves a pure translation whose
# conservation property is exact.
[LinearFVKernels]
  [phase_time]
    type = LinearFVTimeDerivative
    variable = phase_2
  []
  [phase_advection]
    type = LinearFVScalarAdvection
    variable = phase_2
    advected_interp_method_name = upwind
    rhie_chow_user_object = ins_rhie_chow_interpolator
    u_slip = 0
    v_slip = ${slip}
  []
[]

# A wall may legitimately pin the phase fraction. This is what the lid driven case does, and it is
# the configuration in which a slip applied on a boundary face actually reaches the assembly: with
# no boundary condition at all the advection kernel contributes nothing on that face and the guard
# would be untestable.
[LinearFVBCs]
  [top_phase_2]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = phase_2
    boundary = 'top'
    functor = 1.0
  []
  [bottom_phase_2]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = phase_2
    boundary = 'bottom'
    functor = 0.0
  []
[]

[Functions]
  [stratified]
    # Non-uniform, so that the slip actually transports something
    type = ParsedFunction
    expression = 'if(y > 0.05, 0.7, 0.3)'
  []
[]

[Executioner]
  type = PIMPLE
  rhie_chow_user_object = 'ins_rhie_chow_interpolator'

  num_steps = 4
  dt = 0.05

  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  active_scalar_systems = 'phi_system'
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
  continue_on_max_its = true

  pin_pressure = true
  pressure_pin_value = 0.0
  pressure_pin_point = '0.0 0.0 0.0'
[]

[Postprocessors]
  [phase_2_integral]
    type = ElementIntegralVariablePostprocessor
    variable = phase_2
    execute_on = 'INITIAL TIMESTEP_END'
  []
  # The assertion: this must stay at zero
  [phase_2_integral_drift]
    type = ChangeOverTimePostprocessor
    postprocessor = phase_2_integral
    change_with_respect_to_initial = true
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [min_phase_2]
    type = ElementExtremeValue
    variable = phase_2
    value_type = min
  []
  [max_phase_2]
    type = ElementExtremeValue
    variable = phase_2
    value_type = max
  []
[]

[Outputs]
  [out]
    type = CSV
    execute_on = 'FINAL'
  []
[]
