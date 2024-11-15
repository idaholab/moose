################################################################################
# MATERIAL PROPERTIES
################################################################################
rho = 3279.
T_0 = 875.0
mu = 1.
k_cond = 38.0
cp = 640.
alpha = 3.26e-4

walls = 'right left top bottom'

[GlobalParams]
  rhie_chow_user_object = 'ins_rhie_chow_interpolator'
  advected_interp_method = 'upwind'
  u = superficial_vel_x
  v = superficial_vel_y
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system energy_system'
  previous_nl_solution_required = true
[]

################################################################################
# GEOMETRY
################################################################################

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 30
    ny = 30
  []
[]

################################################################################
# EQUATIONS: VARIABLES, KERNELS & BCS
################################################################################

[UserObjects]
  [ins_rhie_chow_interpolator]
    type = RhieChowMassFlux
    u = superficial_vel_x
    v = superficial_vel_y
    pressure = pressure
    rho = ${rho}
    p_diffusion_kernel = p_diffusion
  []
[]

[Variables]
  [superficial_vel_x]
    type = MooseLinearVariableFVReal
    solver_sys = u_system
  []
  [superficial_vel_y]
    type = MooseLinearVariableFVReal
    solver_sys = v_system
  []
  [pressure]
    type = MooseLinearVariableFVReal
    initial_condition = 0
    solver_sys = pressure_system
  []
  [T_fluid]
    type = MooseLinearVariableFVReal
    solver_sys = energy_system
    initial_condition = 875
  []
[]

[LinearFVKernels]
  [u_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = superficial_vel_x
    mu = ${mu}
    momentum_component = 'x'
    use_nonorthogonal_correction = true
  []
  [u_pressure]
    type = LinearFVMomentumPressure
    variable = superficial_vel_x
    pressure = pressure
    momentum_component = 'x'
  []
  [u_buoyancy]
    type = LinearFVMomentumBoussinesq
    variable = superficial_vel_x
    T_fluid = T_fluid
    gravity = '0 -9.81 0'
    rho = ${rho}
    ref_temperature = ${T_0}
    alpha_name = ${alpha}
    momentum_component = 'x'
  []

  [v_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = superficial_vel_y
    mu = ${mu}
    momentum_component = 'y'
    use_nonorthogonal_correction = true
  []
  [v_pressure]
    type = LinearFVMomentumPressure
    variable = superficial_vel_y
    pressure = pressure
    momentum_component = 'y'
  []
  [v_buoyancy]
    type = LinearFVMomentumBoussinesq
    variable = superficial_vel_y
    T_fluid = T_fluid
    gravity = '0 -9.81 0'
    rho = ${rho}
    ref_temperature = ${T_0}
    alpha_name = ${alpha}
    momentum_component = 'y'
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

   ####### FUEL ENERGY EQUATION #######

  [heat_advection]
    type = LinearFVEnergyAdvection
    variable = T_fluid
    cp = ${cp}
  []
  [conduction]
    type = LinearFVDiffusion
    variable = T_fluid
    diffusion_coeff = ${fparse k_cond}
  []
[]


[LinearFVBCs]
  [no-slip-u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = superficial_vel_x
    boundary = ${walls}
    functor = 0
  []
  [no-slip-v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = superficial_vel_y
    boundary = ${walls}
    functor = 0
  []
  [T_cold]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T_fluid
    boundary = 'right'
    functor = 870.0
  []
  [T_hot]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T_fluid
    boundary = 'left'
    functor = 880.0
  []
  [T_all]
    type = LinearFVAdvectionDiffusionExtrapolatedBC
    variable = T_fluid
    boundary = 'top bottom'
    use_two_term_expansion = false
  []
  [pressure-extrapolation]
    type = LinearFVExtrapolatedPressureBC
    boundary = ${walls}
    variable = pressure
    use_two_term_expansion = false
  []
[]

[FunctorMaterials]
  [constant_functors]
    type = GenericFunctorMaterial
    prop_names = 'cp alpha_b'
    prop_values = '${cp} ${alpha}'
  []
[]

################################################################################
# EXECUTION / SOLVE
################################################################################

[Executioner]
  type = SIMPLE
  momentum_l_abs_tol = 1e-11
  pressure_l_abs_tol = 1e-11
  energy_l_abs_tol = 1e-11
  momentum_l_tol = 0
  pressure_l_tol = 0
  energy_l_tol = 0
  rhie_chow_user_object = 'ins_rhie_chow_interpolator'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  energy_system = 'energy_system'
  momentum_equation_relaxation = 0.3
  pressure_variable_relaxation = 0.7
  energy_equation_relaxation = 0.95
  num_iterations = 3000
  pressure_absolute_tolerance = 1e-8
  momentum_absolute_tolerance = 1e-8
  energy_absolute_tolerance = 1e-8
  print_fields = false
  momentum_l_max_its = 300

  pin_pressure = true
  pressure_pin_value = 0.0
  pressure_pin_point = '0.5 0.0 0.0'

  # momentum_petsc_options = '-ksp_monitor'
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'

  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'

  energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  energy_petsc_options_value = 'hypre boomeramg'
[]

################################################################################
# SIMULATION OUTPUTS
################################################################################

[Outputs]
  exodus = true
[]

