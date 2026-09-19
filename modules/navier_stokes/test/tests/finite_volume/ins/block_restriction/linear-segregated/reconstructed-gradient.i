mu = 1
rho = 1
flow_block = 2
restricted_blocks = '${flow_block}'
inlet_speed = 1

[Mesh]
  [blocks]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1 1'
    dy = '1'
    ix = '1 2 1'
    iy = '1'
    subdomain_id = '1 ${flow_block} 3'
  []
  [internal_inlet]
    type = SideSetsBetweenSubdomainsGenerator
    input = blocks
    primary_block = ${flow_block}
    paired_block = 1
    new_boundary = internal_inlet
  []
  [internal_outlet]
    type = SideSetsBetweenSubdomainsGenerator
    input = internal_inlet
    primary_block = ${flow_block}
    paired_block = 3
    new_boundary = internal_outlet
  []
  [delete_outside_blocks]
    type = BlockDeletionGenerator
    input = internal_outlet
    block = '1 3'
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
    block = ${restricted_blocks}
  []
[]

[Variables]
  [vel_x]
    type = MooseLinearVariableFVReal
    initial_condition = 0
    solver_sys = u_system
    block = ${restricted_blocks}
  []
  [vel_y]
    type = MooseLinearVariableFVReal
    solver_sys = v_system
    block = ${restricted_blocks}
  []
  [pressure]
    type = MooseLinearVariableFVReal
    solver_sys = pressure_system
    block = ${restricted_blocks}
    gradient_method = reconstructed
  []
[]

[FVGradientMethods]
  [reconstructed]
    type = FVReconstructedPressureGradient
    base_gradient_method = green-gauss
    # This moderate relaxation exposes the orientation feedback within the short regression.
    gradient_relaxation = 0.5
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
    boundary = internal_outlet
    variable = pressure
    functor = 0
  []
  [outlet-u]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = vel_x
    use_two_term_expansion = false
    boundary = internal_outlet
  []
  [outlet-v]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = vel_y
    use_two_term_expansion = false
    boundary = internal_outlet
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
  momentum_l_abs_tol = 1e-12
  pressure_l_abs_tol = 1e-12
  momentum_l_tol = 0
  pressure_l_tol = 0
  rhie_chow_user_object = rc
  momentum_systems = 'u_system v_system'
  pressure_system = pressure_system
  momentum_equation_relaxation = 0.8
  pressure_variable_relaxation = 0.3
  # Three iterations let the first reconstructed candidate feed a subsequent momentum predictor.
  num_iterations = 3
  continue_on_max_its = true
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
