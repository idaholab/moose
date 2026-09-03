mu = 1
rho = 1
pressure_gradient_method = reconstructed

[Mesh]
  [channel]
    type = CartesianMeshGenerator
    dim = 2
    # Twenty cells per section resolve the interface-localized oscillation without making this
    # two-solve comparison unnecessarily expensive.
    dx = '1 1 1'
    ix = '20 20 20'
    dy = 1
    iy = 3
    subdomain_id = '0 1 2'
  []
  [blocks]
    type = RenameBlockGenerator
    input = channel
    old_block = '0 1 2'
    new_block = 'upstream forced downstream'
  []
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system'
  previous_nl_solution_required = true
[]

[UserObjects]
  [rc]
    type = RhieChowMassFlux
    u = vel_x
    v = vel_y
    pressure = pressure
    rho = ${rho}
    p_diffusion_kernel = p_diffusion
    enforce_coupling_pressure_gradient_identity = true
  []
[]

[Variables]
  [vel_x]
    type = MooseLinearVariableFVReal
    initial_condition = 1
    solver_sys = u_system
  []
  [vel_y]
    type = MooseLinearVariableFVReal
    initial_condition = 0
    solver_sys = v_system
  []
  [pressure]
    type = MooseLinearVariableFVReal
    initial_condition = 0
    solver_sys = pressure_system
    gradient_method = ${pressure_gradient_method}
  []
[]

[FVGradientMethods]
  [reconstructed]
    type = FVReconstructedPressureGradient
    base_gradient_method = green-gauss
    # This is the relaxation used by the reconstructed-gradient channel regression tests.
    gradient_relaxation = 0.1
  []
[]

[FVInterpolationMethods]
  [upwind]
    type = FVAdvectedUpwind
  []
[]

[LinearFVKernels]
  [u_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_x
    advected_interp_method_name = upwind
    mu = ${mu}
    u = vel_x
    v = vel_y
    momentum_component = x
    rhie_chow_user_object = rc
    use_nonorthogonal_correction = false
  []
  [v_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_y
    advected_interp_method_name = upwind
    mu = ${mu}
    u = vel_x
    v = vel_y
    momentum_component = y
    rhie_chow_user_object = rc
    use_nonorthogonal_correction = false
  []
  [u_pressure]
    type = LinearFVMomentumPressure
    variable = vel_x
    pressure = pressure
    momentum_component = x
  []
  [v_pressure]
    type = LinearFVMomentumPressure
    variable = vel_y
    pressure = pressure
    momentum_component = y
  []
  [middle_force]
    type = LinearFVSource
    variable = vel_x
    # The large source makes the pressure-gradient jump dominate truncation error and visibly
    # excites the unsupported cell-velocity mode when reconstruction is disabled.
    source_density = 1e4
    block = forced
  []
  [p_diffusion]
    type = LinearFVPressureCorrectionDiffusion
    variable = pressure
    diffusion_tensor = Ainv
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
  [inlet_u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = left
    variable = vel_x
    functor = 1
  []
  [inlet_v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = left
    variable = vel_y
    functor = 0
  []
  [symmetry_u]
    type = LinearFVVelocitySymmetryBC
    boundary = 'top bottom'
    variable = vel_x
    momentum_component = x
    u = vel_x
    v = vel_y
  []
  [symmetry_v]
    type = LinearFVVelocitySymmetryBC
    boundary = 'top bottom'
    variable = vel_y
    momentum_component = y
    u = vel_x
    v = vel_y
  []
  [pressure_flux]
    type = LinearFVPressureFluxBC
    boundary = left
    variable = pressure
    HbyA_flux = HbyA
    Ainv = Ainv
    u = vel_x
    v = vel_y
    rho = ${rho}
  []
  [pressure_symmetry]
    type = LinearFVPressureSymmetryBC
    boundary = 'top bottom'
    variable = pressure
    HbyA_flux = HbyA
  []
  [outlet_u]
    type = LinearFVAdvectionDiffusionOutflowBC
    boundary = right
    variable = vel_x
    use_two_term_expansion = false
  []
  [outlet_v]
    type = LinearFVAdvectionDiffusionOutflowBC
    boundary = right
    variable = vel_y
    use_two_term_expansion = false
  []
  [outlet_p]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = right
    variable = pressure
    functor = 0
  []
[]

[Postprocessors]
  [upstream_vel_x]
    type = ElementAverageValue
    variable = vel_x
    block = upstream
    execute_on = FINAL
  []
  [forced_vel_x]
    type = ElementAverageValue
    variable = vel_x
    block = forced
    execute_on = FINAL
  []
  [downstream_vel_x]
    type = ElementAverageValue
    variable = vel_x
    block = downstream
    execute_on = FINAL
  []
[]

[Executioner]
  type = SIMPLE
  momentum_l_abs_tol = 1e-10
  pressure_l_abs_tol = 1e-10
  momentum_l_tol = 0
  pressure_l_tol = 0
  rhie_chow_user_object = rc
  momentum_systems = 'u_system v_system'
  pressure_system = pressure_system
  momentum_equation_relaxation = 0.8
  pressure_variable_relaxation = 0.3
  # Five hundred iterations allow the slowly relaxed reconstructed feedback to reach the same
  # uniform steady field as the standard gradient. The reconstructed momentum residual is
  # cancellation-dominated at the source jumps and does not satisfy the generic normalized monitor.
  num_iterations = 500
  continue_on_max_its = true
  # Relative to the 1e4 source, this requires the ordinary outer residuals to fall by eleven orders
  # of magnitude while remaining above platform-level roundoff stagnation.
  pressure_absolute_tolerance = 1e-7
  momentum_absolute_tolerance = 1e-7
  momentum_petsc_options_iname = '-pc_type'
  momentum_petsc_options_value = lu
  pressure_petsc_options_iname = '-pc_type'
  pressure_petsc_options_value = lu
  print_fields = false
[]

[Outputs]
  [csv]
    type = CSV
    execute_on = FINAL
  []
  exodus = true
[]
