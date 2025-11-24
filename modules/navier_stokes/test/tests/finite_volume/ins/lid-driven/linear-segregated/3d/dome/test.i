ilet_width = 0.5472
ilet_length = 0.4064
ilet_area = '${fparse ilet_width * ilet_length}'
volumetric_flow_rate = 0.31478894915 # m3/s
velocity_inlet_magnitude = ${fparse volumetric_flow_rate / ilet_area}
# Make this negative so that it's the opposite direction of the normal vector
velocity_diri_condition = '${fparse -velocity_inlet_magnitude}'

# air
rho = 1.177
mu = 1.846e-5
k = .0262
cp = 1006
beta = 3.33e-3
alpha = ${fparse k / (cp * rho)}
nu = ${fparse mu / rho}

# Dimensionless numbers
L = 10
Re = ${fparse L * velocity_inlet_magnitude / nu}
Pr = ${fparse cp * mu / k}

# mixed laminar-turbulent forced convection flat plate correlation
Re_crit = 5e5
Nu = ${fparse (.037 * Re^(4/5) - .664 * Re_crit^(1/2)) * Pr^(1/3)}
h = ${fparse Nu * k / L}

T_0 = 300.0
T_hot = 373
T_cold = ${T_0}
initial_dt = .4

[GlobalParams]
  rhie_chow_user_object = 'rc'
  advected_interp_method = 'upwind'
[]

[Mesh]
  [file]
    type = FileMeshGenerator
    file = zach-mesh_in.e
  []
  uniform_refine = 0
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
  [inlet_x]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'inlet'
    functor = ${velocity_diri_condition}
    variable = vel_x
    normal_component = 'x'
  []
  [inlet_y]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'inlet'
    functor = ${velocity_diri_condition}
    variable = vel_y
    normal_component = 'y'
  []
  [inlet_z]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'inlet'
    functor = ${velocity_diri_condition}
    variable = vel_z
    normal_component = 'z'
  []
  [inlet_T]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'inlet'
    functor = ${T_cold}
    variable = T_fluid
  []

  [outlet_p]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'outlet'
    variable = pressure
    functor = 0
  []
  [outlet_x]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = vel_x
    use_two_term_expansion = true
    boundary = outlet
  []
  [outlet_y]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = vel_y
    use_two_term_expansion = true
    boundary = outlet
  []
  [outlet_z]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = vel_z
    use_two_term_expansion = true
    boundary = outlet
  []
  [outlet_T]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = T_fluid
    use_two_term_expansion = true
    boundary = outlet
  []

  [no_slip_x]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = vel_x
    boundary = 'air_box_boundary air_wall_boundary air_floor_boundary'
    functor = 0
  []
  [no_slip_y]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = vel_y
    boundary = 'air_box_boundary air_wall_boundary air_floor_boundary'
    functor = 0
  []
  [no_slip_z]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = vel_z
    boundary = 'air_box_boundary air_wall_boundary air_floor_boundary'
    functor = 0
  []
  [pressure-flux]
    type = LinearFVPressureFluxBC
    boundary = 'air_box_boundary air_wall_boundary air_floor_boundary'
    variable = pressure
    HbyA_flux = HbyA
    Ainv = Ainv
  []
  [T_adiabatic]
    type = LinearFVAdvectionDiffusionFunctorNeumannBC
    boundary = 'air_floor_boundary'
    functor = 0
    variable = T_fluid
  []
  [T_hot]
    type = LinearFVConvectiveHeatTransferBC
    boundary = 'air_box_boundary'
    variable = T_fluid
    h = ${h}
    T_fluid = T_fluid
    T_solid = ${T_hot}
  []
  [T_cold]
    type = LinearFVConvectiveHeatTransferBC
    boundary = 'air_wall_boundary'
    T_solid = ${T_cold}
    T_fluid = T_fluid
    variable = T_fluid
    h = ${h}
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
  num_iterations = 100
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

  num_steps = 30000
  num_piso_iterations = 0

  [TimeStepper]
    type = PostprocessorDT
    postprocessor = new_dt_for_unity_cfl
    dt = ${initial_dt}
    scale = 1
  []

  scheme = bdf2
[]

[Outputs]
  [nemesis]
    type = Nemesis
    time_step_interval = 20
  []
  csv = true
  checkpoint = true
  perf_graph = true
  print_nonlinear_residuals = false
  print_linear_residuals = true
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
  [vel_avg]
    type = ElementAverageValue
    variable = vel_mag
  []
  [T_max]
    type = ElementExtremeValue
    variable = T_fluid
    value_type = max
  []
  [T_avg]
    type = ElementAverageValue
    variable = T_fluid
  []
  [T_boundary_max]
    type = SideExtremeValue
    variable = T_fluid
    value_type = max
    boundary = 'air_wall_boundary'
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
  [h]
    type = AverageElementSize
    execute_on = 'initial'
  []
  [t_diff]
    type = ParsedPostprocessor
    expression = 'h^2 / alpha'
    pp_names = 'h'
    constant_names = 'alpha'
    constant_expressions = '${alpha}'
    execute_on = 'initial'
  []
[]
