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

T_0 = 300
T_hot = 301
T_cold = 300
l = 10
expt_l_factor = 0.5
expt_l = '${fparse expt_l_factor * l}'
p_pin_loc = '${fparse 0.95 * l}'
n = 100
h = '${fparse l / n}'
initial_dt = 15

[GlobalParams]
  rhie_chow_user_object = 'rc'
  advected_interp_method = 'upwind'
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = ${l}
    ymin = 0
    ymax = ${l}
    nx = ${n}
    ny = ${n}
  []
  [left_bottom_corner]
    type = SubdomainBoundingBoxGenerator
    block_id = 1
    bottom_left = '0 0 0'
    input = gen
    top_right = '${expt_l} ${expt_l} 0'
  []
  [new_sideset]
    type = SideSetsBetweenSubdomainsGenerator
    input = left_bottom_corner
    new_boundary = experiment
    paired_block = 1
    primary_block = 0
  []
  [remove]
    type = BlockDeletionGenerator
    input = new_sideset
    block = '1'
  []
  coord_type = 'RZ'
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system energy_system'
  previous_nl_solution_required = true
[]

[AuxVariables]
  [vel_z]
    family = MONOMIAL
    order = CONSTANT
  []
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
    execute_on = 'TIMESTEP_END'
  []
[]

[UserObjects]
  [rc]
    type = RhieChowMassFlux
    u = vel_x
    v = vel_y
    pressure = pressure
    rho = ${rho}
    p_diffusion_kernel = p_diffusion
    body_force_kernel_names = "u_buoyancy; v_buoyancy"
    pressure_projection_method = consistent
    # pressure_projection_method = standard
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
  [pressure]
    type = MooseLinearVariableFVReal
    solver_sys = pressure_system
    initial_condition = 0
  []
  [T_fluid]
    type = MooseLinearVariableFVReal
    solver_sys = energy_system
    initial_condition = ${T_cold}
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
    mu = ${mu}
    u = vel_x
    v = vel_y
    momentum_component = 'x'
    use_nonorthogonal_correction = false
  []
  [v_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_y
    mu = ${mu}
    u = vel_x
    v = vel_y
    momentum_component = 'y'
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
  [u_buoyancy]
    type = LinearFVMomentumBoussinesq
    variable = vel_x
    T_fluid = T_fluid
    gravity = '0 -9.8 0'
    rho = ${rho}
    ref_temperature = ${T_0}
    alpha_name = ${beta}
    momentum_component = 'x'
  []
  [v_buoyancy]
    type = LinearFVMomentumBoussinesq
    variable = vel_y
    T_fluid = T_fluid
    gravity = '0 -9.8 0'
    rho = ${rho}
    ref_temperature = ${T_0}
    alpha_name = ${beta}
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
  []
[]

[LinearFVBCs]
  [no_slip_x]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = vel_x
    boundary = 'top right bottom experiment'
    functor = 0
  []
  [no_slip_y]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = vel_y
    boundary = 'top right bottom experiment'
    functor = 0
  []
  [symmetry-u]
    type = LinearFVVelocitySymmetryBC
    variable = vel_x
    momentum_component = x
    u = vel_x
    v = vel_y
    boundary = 'left'
  []
  [symmetry-v]
    type = LinearFVVelocitySymmetryBC
    variable = vel_y
    momentum_component = y
    u = vel_x
    v = vel_y
    boundary = 'left'
  []
  [symmetry-p]
    type = LinearFVPressureSymmetryBC
    variable = pressure
    boundary = 'left'
    HbyA_flux = 'HbyA' # Functor created in the RhieChowMassFlux UO
  []
  [pressure-flux]
    type = LinearFVPressureFluxBC
    boundary = 'right top bottom experiment'
    variable = pressure
    HbyA_flux = HbyA
    Ainv = Ainv
  []
  [T_symmetry]
    type = LinearFVAdvectionDiffusionScalarSymmetryBC
    boundary = 'left'
    variable = T_fluid
  []
  [T_adiabatic]
    type = LinearFVAdvectionDiffusionFunctorNeumannBC
    boundary = 'bottom'
    functor = 0
    variable = T_fluid
  []
  [T_hot]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'experiment'
    functor = ${T_hot}
    variable = T_fluid
  []
  [T_cold]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'top right'
    functor = ${T_cold}
    variable = T_fluid
  []
[]

[Executioner]
  type = PIMPLE
  momentum_l_abs_tol = 1e-10
  pressure_l_abs_tol = 1e-10
  energy_l_abs_tol = 1e-10
  momentum_l_tol = 0
  pressure_l_tol = 0
  energy_l_tol = 0
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  energy_system = 'energy_system'
  momentum_equation_relaxation = 0.9
  pressure_variable_relaxation = 0.9
  energy_equation_relaxation = 0.9
  num_iterations = 1500
  pressure_absolute_tolerance = 1e-7
  momentum_absolute_tolerance = 1e-7
  energy_absolute_tolerance = 1e-7
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  energy_petsc_options_value = 'hypre boomeramg'
  print_fields = false

  pin_pressure = true
  pressure_pin_value = 0.0
  pressure_pin_point = '${p_pin_loc} ${p_pin_loc} 0.0'

  num_steps = 10000
  num_piso_iterations = 0

  dtmax = ${initial_dt}

  [TimeStepper]
    type = PostprocessorDT
    postprocessor = new_dt_for_unity_cfl
    dt = ${initial_dt}
    scale = 1
  []
[]

[Outputs]
  exodus = true
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
    l = ${l}
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
    constant_expressions = '${h}'
    pp_names = 'vel_max dt'
  []
  # [new_dt_for_unity_cfl]
  #   type = ParsedPostprocessor
  #   expression = 'dt / cfl'
  #   pp_names = 'dt cfl'
  # []
[]
