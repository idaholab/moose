mu = 2.6
rho = 1.0
advected_interp_method = 'average'
cp = 300
k = 10
alpha_b = 1e-4

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1.'
    dy = '0.2'
    ix = '10'
    iy = '5'
  []
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system energy_system'
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
  []
[]

[Variables]
  [vel_x]
    type = MooseLinearVariableFVReal
    initial_condition = 0.5
    solver_sys = u_system
  []
  [vel_y]
    type = MooseLinearVariableFVReal
    solver_sys = v_system
    initial_condition = 0.0
  []
  [pressure]
    type = MooseLinearVariableFVReal
    solver_sys = pressure_system
    initial_condition = 0.2
  []
  [T]
    type = MooseLinearVariableFVReal
    solver_sys = energy_system
    initial_condition = 300
  []
[]

[LinearFVKernels]
  [u_time]
    type = LinearFVTimeDerivative
    variable = vel_x
    factor = ${rho}
  []
  [v_time]
    type = LinearFVTimeDerivative
    variable = vel_y
    factor = ${rho}
  []
  [u_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_x
    advected_interp_method = ${advected_interp_method}
    mu = ${mu}
    u = vel_x
    v = vel_y
    momentum_component = 'x'
    rhie_chow_user_object = 'rc'
    use_nonorthogonal_correction = false
  []
  [v_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_y
    advected_interp_method = ${advected_interp_method}
    mu = ${mu}
    u = vel_x
    v = vel_y
    momentum_component = 'y'
    rhie_chow_user_object = 'rc'
    use_nonorthogonal_correction = false
  []
  [u_pressure]
    type = LinearFVMomentumPressure
    variable = vel_x
    pressure = pressure
    momentum_component = 'x'
  []
  [v_pressure]
    type = LinearFVMomentumPressure
    variable = vel_y
    pressure = pressure
    momentum_component = 'y'
  []
  [u_boussinesq]
    type = LinearFVMomentumBoussinesq
    variable = vel_x
    rho = ${rho}
    gravity = '0 -9.81 0'
    alpha_name = ${alpha_b}
    ref_temperature = 300.0
    T_fluid = T
    momentum_component = 'x'
  []
  [v_boussinesq]
    type = LinearFVMomentumBoussinesq
    variable = vel_y
    rho = ${rho}
    gravity = '0 -9.81 0'
    alpha_name = ${alpha_b}
    ref_temperature = 300.0
    T_fluid = T
    momentum_component = 'y'
  []

  [p_diffusion]
    type = LinearFVAnisotropicDiffusion
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

  [h_time]
    type = LinearFVTimeDerivative
    variable = T
    factor = ${fparse rho*cp}
  []
  [h_advection]
    type = LinearFVEnergyAdvection
    variable = T
    cp = ${cp}
    advected_interp_method = ${advected_interp_method}
    rhie_chow_user_object = 'rc'
  []
  [conduction]
    type = LinearFVDiffusion
    variable = T
    diffusion_coeff = ${k}
    use_nonorthogonal_correction = false
  []
[]

[FunctorMaterials]
  [constant_functors]
    type = GenericFunctorMaterial
    prop_names = 'cp alpha_b'
    prop_values = '${cp} ${alpha_b}'
  []
[]

[LinearFVBCs]
  [inlet-u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left'
    variable = vel_x
    functor = '1.1'
  []
  [inlet-v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left'
    variable = vel_y
    functor = '0.0'
  []
  [walls-u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'top bottom'
    variable = vel_x
    functor = 0.0
  []
  [walls-v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'top bottom'
    variable = vel_y
    functor = 0.0
  []
  [outlet_p]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'right'
    variable = pressure
    functor = 1.4
  []
  [outlet_u]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = vel_x
    use_two_term_expansion = false
    boundary = right
  []
  [outlet_v]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = vel_y
    use_two_term_expansion = false
    boundary = right
  []
  [inlet_top_T]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T
    functor = 300.0
    boundary = 'left top'
  []
  [bottom_T]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T
    functor = wall-temperature
    boundary = bottom
  []
  [outlet_T]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = T
    use_two_term_expansion = false
    boundary = right
  []
[]

[Functions]
  [wall-temperature]
    type = ParsedFunction
    expression = '350 + 50 * sin(6.28*t)'
  []
[]

[Executioner]
  type = PIMPLE
  momentum_l_abs_tol = 1e-12
  pressure_l_abs_tol = 1e-12
  energy_l_abs_tol = 1e-12
  momentum_l_tol = 1e-12
  pressure_l_tol = 1e-12
  energy_l_tol = 1e-12
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  energy_system = 'energy_system'
  momentum_equation_relaxation = 0.8
  pressure_variable_relaxation = 0.3
  energy_equation_relaxation = 0.9
  num_iterations = 100
  pressure_absolute_tolerance = 1e-11
  momentum_absolute_tolerance = 1e-11
  energy_absolute_tolerance = 1e-11
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  energy_petsc_options_value = 'hypre boomeramg'
  print_fields = false
  continue_on_max_its = true
  dt = 0.01
  num_steps = 6
  num_piso_iterations = 0
[]


[Outputs]
  exodus = true
[]
