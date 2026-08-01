# Unit properties and speed give the exact constant plug-flow solution used by the CSV comparisons.
mu = 1
rho = 1
flow_block = 2
inlet_speed = 1

[Mesh]
  [blocks]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1'
    dy = '1'
    ix = '1 2'
    iy = '1'
    subdomain_id = '1 ${flow_block}'
  []
  [internal_inlet]
    type = SideSetsBetweenSubdomainsGenerator
    input = blocks
    primary_block = ${flow_block}
    paired_block = 1
    new_boundary = internal_inlet
  []
  [delete_inlet_block]
    type = BlockDeletionGenerator
    input = internal_inlet
    block = 1
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
    block = ${flow_block}
  []
[]

[Variables]
  [vel_x]
    type = MooseLinearVariableFVReal
    initial_condition = ${inlet_speed}
    solver_sys = u_system
    block = ${flow_block}
  []
  [vel_y]
    type = MooseLinearVariableFVReal
    solver_sys = v_system
    block = ${flow_block}
  []
  [pressure]
    type = MooseLinearVariableFVReal
    solver_sys = pressure_system
    block = ${flow_block}
  []
[]

[FVInterpolationMethods]
  [average]
    type = FVGeometricAverage
  []
[]

[LinearFVKernels]
  [u_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_x
    advected_interp_method_name = average
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
    advected_interp_method_name = average
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
  [prescribed-u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'internal_inlet top bottom'
    variable = vel_x
    functor = ${inlet_speed}
  []
  [prescribed-v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'internal_inlet top bottom'
    variable = vel_y
    functor = 0
  []
  [pressure-flux]
    type = LinearFVPressureFluxBC
    boundary = 'internal_inlet top bottom'
    variable = pressure
    HbyA_flux = HbyA
    Ainv = Ainv
    u = vel_x
    v = vel_y
    rho = ${rho}
  []
  [outlet-pressure]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = right
    variable = pressure
    functor = 0
  []
  [outlet-u]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = vel_x
    use_two_term_expansion = false
    boundary = right
  []
  [outlet-v]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = vel_y
    use_two_term_expansion = false
    boundary = right
  []
[]

[Functions]
  [exact-u]
    type = ParsedFunction
    expression = ${inlet_speed}
  []
  [exact-zero]
    type = ParsedFunction
    expression = 0
  []
[]

[Postprocessors]
  [inlet-mass-flow]
    type = VolumetricFlowRate
    boundary = internal_inlet
    vel_x = vel_x
    vel_y = vel_y
    advected_quantity = ${rho}
    rhie_chow_user_object = rc
  []
  [u-error]
    type = ElementL2FunctorError
    approximate = vel_x
    exact = exact-u
    block = ${flow_block}
  []
  [v-error]
    type = ElementL2FunctorError
    approximate = vel_y
    exact = exact-zero
    block = ${flow_block}
  []
  [pressure-error]
    type = ElementL2FunctorError
    approximate = pressure
    exact = exact-zero
    block = ${flow_block}
  []
[]

[Executioner]
  type = SIMPLE
  # Tight algebraic tolerances make the reported errors sensitive to the internal-boundary sign.
  momentum_l_abs_tol = 1e-12
  pressure_l_abs_tol = 1e-12
  momentum_l_tol = 0
  pressure_l_tol = 0
  rhie_chow_user_object = rc
  momentum_systems = 'u_system v_system'
  pressure_system = pressure_system
  # These standard SIMPLE relaxation factors provide stable convergence for the tiny test mesh.
  momentum_equation_relaxation = 0.8
  pressure_variable_relaxation = 0.3
  # Fifty iterations is a conservative cap; the exact initialized solution converges much sooner.
  num_iterations = 50
  # These nonlinear tolerances keep the final CSV errors below the comparison threshold.
  pressure_absolute_tolerance = 1e-10
  momentum_absolute_tolerance = 1e-10
  momentum_petsc_options_iname = '-pc_type'
  momentum_petsc_options_value = 'lu'
  pressure_petsc_options_iname = '-pc_type'
  pressure_petsc_options_value = 'lu'
  print_fields = false
[]

[Outputs]
  csv = true
  execute_on = FINAL
[]
