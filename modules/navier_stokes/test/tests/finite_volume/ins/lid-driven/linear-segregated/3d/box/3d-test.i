# air
# rho = 1.177
# mu = 1.846e-5
# k = .0262
# cp = 1006
# beta = 3.33e-3
# water
# rho = 997
# mu = 8.55e-4
# k = .613
# cp = 4182
# beta = 2.57e-4
# glycerol-water
# rho = 1110
# mu = 5e-3
# k = .4
# cp = 3500
# beta = 3.8e-4
# glycerol
rho = 1260
mu = 1.41
k = .285
cp = 2410
beta = 5e-4

T_0 = 300.0
T_hot = 301
T_cold = 300
initial_dt = 20

[GlobalParams]
  rhie_chow_user_object = 'rc'
  advected_interp_method = 'upwind'
[]

[Mesh]
  [file]
    type = FileMeshGenerator
    file = remove-solid-blocks_in.e
  []
[]

[Problem]
  linear_sys_names = 'u_system v_system w_system pressure_system energy_system'
  previous_nl_solution_required = true
[]

[AuxVariables]
  [vel_mag]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [vel_mag]
    type = VectorMagnitudeAux
    variable = vel_mag
    x = vel_x
    y = vel_y
    z = vel_z
    execute_on = 'TIMESTEP_END'
  []
[]

[UserObjects]
  [rc]
    type = RhieChowMassFlux
    u = vel_x
    v = vel_y
    w = vel_z
    pressure = pressure
    rho = ${rho}
    p_diffusion_kernel = p_diffusion
    body_force_kernel_names = "; ; w_buoyancy"
    pressure_projection_method = consistent
  []
[]

[Variables]
  [vel_x]
    type = MooseLinearVariableFVReal
    initial_condition = 0.0
    solver_sys = u_system
  []
  [vel_y]
    type = MooseLinearVariableFVReal
    initial_condition = 0.0
    solver_sys = v_system
  []
  [vel_z]
    type = MooseLinearVariableFVReal
    initial_condition = 0.0
    solver_sys = w_system
  []
  [pressure]
    type = MooseLinearVariableFVReal
    solver_sys = pressure_system
    initial_condition = 0
  []
  [T_fluid]
    type = MooseLinearVariableFVReal
    solver_sys = energy_system
    initial_condition = ${T_0}
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
  [w_time]
    type = LinearFVTimeDerivative
    variable = vel_z
    factor = ${rho}
  []
  [u_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_x
    mu = ${mu}
    u = vel_x
    v = vel_y
    w = vel_z
    momentum_component = 'x'
    use_nonorthogonal_correction = true
  []
  [v_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_y
    mu = ${mu}
    u = vel_x
    v = vel_y
    w = vel_z
    momentum_component = 'y'
    use_nonorthogonal_correction = true
  []
  [w_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_z
    mu = ${mu}
    u = vel_x
    v = vel_y
    w = vel_z
    momentum_component = 'z'
    use_nonorthogonal_correction = true
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
  [w_pressure]
    type = LinearFVMomentumPressure
    variable = vel_z
    pressure = pressure
    momentum_component = 'z'
  []
  [w_buoyancy]
    type = LinearFVMomentumBoussinesq
    variable = vel_z
    T_fluid = T_fluid
    gravity = '0 0 -9.8'
    rho = ${rho}
    ref_temperature = ${T_0}
    alpha_name = ${beta}
    momentum_component = 'z'
  []
  [p_diffusion]
    type = LinearFVAnisotropicDiffusion
    variable = pressure
    diffusion_tensor = Ainv
    use_nonorthogonal_correction = true
  []
  [HbyA_divergence]
    type = LinearFVDivergence
    variable = pressure
    face_flux = HbyA
    force_boundary_execution = true
  []
  [heat_time]
    type = LinearFVTimeDerivative
    variable = T_fluid
    factor = '${fparse rho*cp}'
  []
  [heat_advection]
    type = LinearFVEnergyAdvection
    variable = T_fluid
    advected_quantity = temperature
    cp = ${cp}
  []
  [heat_conduction]
    type = LinearFVDiffusion
    variable = T_fluid
    diffusion_coeff = ${k}
    use_nonorthogonal_correction = true
  []
[]

[LinearFVBCs]
  [no_slip_x]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = vel_x
    boundary = 'air_box_boundary air_wall_boundary ceiling ground'
    functor = 0
  []
  [no_slip_y]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = vel_y
    boundary = 'air_box_boundary air_wall_boundary ceiling ground'
    functor = 0
  []
  [no_slip_z]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = vel_z
    boundary = 'air_box_boundary air_wall_boundary ceiling ground'
    functor = 0
  []
  [pressure-flux]
    type = LinearFVPressureFluxBC
    boundary = 'air_box_boundary air_wall_boundary ceiling ground'
    variable = pressure
    HbyA_flux = HbyA
    Ainv = Ainv
  []
  [T_adiabatic]
    type = LinearFVAdvectionDiffusionFunctorNeumannBC
    boundary = 'ground'
    functor = 0
    variable = T_fluid
  []
  [T_hot]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'air_box_boundary'
    functor = ${T_hot}
    variable = T_fluid
  []
  [T_cold]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'air_wall_boundary ceiling'
    functor = ${T_cold}
    variable = T_fluid
  []
[]

[Executioner]
  type = PIMPLE
  momentum_l_abs_tol = 1e-8
  pressure_l_abs_tol = 1e-8
  energy_l_abs_tol = 1e-8
  momentum_l_tol = 0
  pressure_l_tol = 0
  energy_l_tol = 0
  momentum_systems = 'u_system v_system w_system'
  pressure_system = 'pressure_system'
  energy_system = 'energy_system'
  momentum_equation_relaxation = 0.80
  pressure_variable_relaxation = 0.95
  energy_equation_relaxation = 0.9
  num_iterations = 1500
  pressure_absolute_tolerance = 1e-4
  momentum_absolute_tolerance = 1e-4
  energy_absolute_tolerance = 1e-4
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type -pc_hypre_boomeramg_agg_nl -pc_hypre_boomeramg_agg_num_paths -pc_hypre_boomeramg_truncfactor -pc_hypre_boomeramg_strong_threshold -pc_hypre_boomeramg_coarsen_type -pc_hypre_boomeramg_interp_type'
  momentum_petsc_options_value = 'hypre boomeramg 4 1 0.1 0.6 HMIS ext+i'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type -pc_hypre_boomeramg_agg_nl -pc_hypre_boomeramg_agg_num_paths -pc_hypre_boomeramg_truncfactor -pc_hypre_boomeramg_strong_threshold -pc_hypre_boomeramg_coarsen_type -pc_hypre_boomeramg_interp_type'
  pressure_petsc_options_value = 'hypre boomeramg 2 1 0.1 0.6 HMIS ext+i'
  energy_petsc_options_iname = '-pc_type -pc_hypre_type -pc_hypre_boomeramg_agg_nl -pc_hypre_boomeramg_agg_num_paths -pc_hypre_boomeramg_truncfactor -pc_hypre_boomeramg_strong_threshold -pc_hypre_boomeramg_coarsen_type -pc_hypre_boomeramg_interp_type'
  energy_petsc_options_value = 'hypre boomeramg 4 1 0.1 0.6 HMIS ext+i'
  print_fields = false

  pin_pressure = true
  pressure_pin_value = 0.0
  pressure_pin_point = '0 0 30'

  num_steps = 15000
  num_piso_iterations = 0

  dt = ${initial_dt}

  # [TimeStepper]
  #   type = PostprocessorDT
  #   postprocessor = new_dt_for_unity_cfl
  #   dt = ${initial_dt}
  #   scale = 1
  # []
[]

[Outputs]
  nemesis = true
  csv = true
  checkpoint = true
  perf_graph = true
  print_nonlinear_residuals = false
  print_linear_residuals = true
  time_step_interval = 20
[]

[Postprocessors]
  [rayleigh]
    type = RayleighNumber
    cp_ave = ${cp}
    gravity_magnitude = 9.8
    k_ave = ${k}
    l = 35
    mu_ave = ${mu}
    rho_ave = ${rho}
    beta = ${beta}
    T_cold = ${T_cold}
    T_hot = ${T_hot}
    execute_on = 'initial timestep_end'
  []
  [vel_max]
    type = ElementExtremeValue
    variable = vel_mag
  []
  [dt]
    type = TimestepSize
  []
  [cfl]
    type = ParsedPostprocessor
    expression = 'vel_max * dt / h'
    constant_names = 'h'
    # Estimate by cbrt
    constant_expressions = '.56'
    pp_names = 'vel_max dt'
  []
  [new_dt_for_unity_cfl]
    type = ParsedPostprocessor
    expression = 'dt / cfl'
    pp_names = 'dt cfl'
  []
[]
