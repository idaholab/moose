
!include header_and_mesh.i

qs = 5

[Problem]
  linear_sys_names = 'u_system v_system pressure_system scalar_system TKE_system TKED_system'
  previous_nl_solution_required = true
[]

[Physics]
  [NavierStokes]
    [FlowSegregated/flow]
      velocity_variable = 'vel_x vel_y'
      pressure_variable = 'pressure'

      # Initial conditions
      initial_velocity = '${bulk_u} 0 0'
      initial_pressure = '1e-8'

      # Material properties
      density = ${rho}
      dynamic_viscosity = ${mu}

      # Boundary conditions
      inlet_boundaries = 'left'
      momentum_inlet_types = 'fixed-velocity'
      momentum_inlet_functors = '${bulk_u} 0'

      wall_boundaries = 'top bottom'
      momentum_wall_types = 'noslip noslip'

      outlet_boundaries = 'right'
      momentum_outlet_types = 'fixed-pressure'
      pressure_functors = '0'

      # Numerical parameters
      include_deviatoric_stress = true
      orthogonality_correction = false
      pressure_two_term_bc_expansion = false
      momentum_two_term_bc_expansion = false
      momentum_advection_interpolation = ${advected_interp_method}
      system_names = 'u_system v_system pressure_system'
    []
    [ScalarTransportSegregated/scalar]
      # turbulence Physics coupling picked up automatically
      passive_scalar_names = 'advected_scalar'

      passive_scalar_inlet_types = 'fixed-value'
      passive_scalar_inlet_functors = '0'

      passive_scalar_source = '${qs}'
      system_names = 'scalar_system'
    []
    [TurbulenceSegregated/k-epsilon]
      # Model
      turbulence_handling = 'k-epsilon'
      tke_name = TKE
      tked_name = TKED
      system_names = 'TKE_system TKED_system'
      scalar_transport_physics = scalar

      initial_tke = ${k_init}
      initial_tked = ${eps_init}

      # Model parameters
      mu_t_ratio_max = 1e20
      sigma_k = ${sigma_k}
      sigma_eps = ${sigma_eps}
      C_pl = 1e10
      C1_eps = ${C1_eps}
      C2_eps = ${C2_eps}
      Sc_t = 0.9

      turbulence_walls = ${walls}
      wall_treatment_eps = ${wall_treatment}
      bulk_wall_treatment = ${bulk_wall_treatment}
      use_nonorthogonal_correction = false
    []
  []
[]

# Boundary conditions are not implemented for turbulent quantities at this time
[LinearFVBCs]
  [inlet_TKE]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left'
    variable = TKE
    functor = '${k_init}'
  []
  [outlet_TKE]
    type = LinearFVAdvectionDiffusionOutflowBC
    boundary = 'right'
    variable = TKE
    use_two_term_expansion = false
  []
  [inlet_TKED]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left'
    variable = TKED
    functor = '${eps_init}'
  []
  [outlet_TKED]
    type = LinearFVAdvectionDiffusionOutflowBC
    boundary = 'right'
    variable = TKED
    use_two_term_expansion = false
  []
[]

[AuxVariables]
  [yplus]
    type = MooseLinearVariableFVReal
  []
[]

[AuxKernels]
  [compute_y_plus]
    type = RANSYPlusAux
    variable = yplus
    tke = TKE
    mu = ${mu}
    rho = ${rho}
    u = vel_x
    v = vel_y
    walls = ${walls}
    wall_treatment = ${wall_treatment}
    execute_on = 'NONLINEAR'
  []
[]

[Executioner]
  type = SIMPLE

  rhie_chow_user_object = 'ins_rhie_chow_interpolator'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  passive_scalar_systems = 'scalar_system'
  turbulence_systems = 'TKE_system TKED_system'

  momentum_l_abs_tol = 1e-14
  pressure_l_abs_tol = 1e-14
  turbulence_l_abs_tol = 1e-14
  passive_scalar_l_abs_tol = 1e-14
  momentum_l_tol = 1e-14
  pressure_l_tol = 1e-14
  passive_scalar_l_tol = 1e-14
  turbulence_l_tol = 1e-14

  momentum_equation_relaxation = 0.7
  pressure_variable_relaxation = 0.3
  passive_scalar_equation_relaxation = 0.8
  turbulence_equation_relaxation = '0.2 0.2'
  turbulence_field_relaxation = '0.2 0.2'
  num_iterations = 1000
  pressure_absolute_tolerance = 1e-12
  momentum_absolute_tolerance = 1e-12
  passive_scalar_absolute_tolerance = 1e-12
  turbulence_absolute_tolerance = '1e-12 1e-12'
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  passive_scalar_petsc_options_iname = '-pc_type -pc_hypre_type'
  passive_scalar_petsc_options_value = 'hypre boomeramg'
  turbulence_petsc_options_iname = '-pc_type -pc_hypre_type'
  turbulence_petsc_options_value = 'hypre boomeramg'

  print_fields = false
  continue_on_max_its = true
[]

variables_to_sample = 'vel_x vel_y pressure advected_scalar TKE TKED'
!include postprocessing.i

# More postprocessor for scalar conservation analysis
[Postprocessors]
  [outlet_scalar]
    type = SideAverageValue
    boundary = right
    variable = advected_scalar
  []
  [S_out]
    type = VolumetricFlowRate
    boundary = 'right'
    vel_x = 'vel_x'
    vel_y = 'vel_y'
    advected_quantity = 'advected_scalar'
    rhie_chow_user_object = 'ins_rhie_chow_interpolator'
  []
  [balance_percent]
    type = ParsedPostprocessor
    expression = '100 * (0 - S_out + 2 * ${L} * ${H} * ${qs}) / S_out'
    pp_names = 'S_out'
  []
[]
