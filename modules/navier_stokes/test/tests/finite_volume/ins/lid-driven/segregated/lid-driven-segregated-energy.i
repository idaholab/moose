mu = 1
rho = 1
k = 0.01
cp = 1
alpha = 1

advected_interp_method = 'upwind'
velocity_interp_method = 'rc'

rayleigh = 1e3
hot_temp = ${rayleigh}
temp_ref = '${fparse hot_temp / 2.}'

pressure_tag = "pressure_grad"

[GlobalParams]
  rhie_chow_user_object = 'rc'
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 10
    ny = 10
  []
[]

[Problem]
  nl_sys_names = 'u_system v_system pressure_system energy_system'
  previous_nl_solution_required = true
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolatorSegregated
    u = vel_x
    v = vel_y
    pressure = pressure
  []
[]

[Variables]
  [vel_x]
    type = INSFVVelocityVariable
    initial_condition = 0.0
    solver_sys = u_system
    two_term_boundary_expansion = false
  []
  [vel_y]
    type = INSFVVelocityVariable
    initial_condition = 0.0
    solver_sys = v_system
    two_term_boundary_expansion = false
  []
  [pressure]
    type = INSFVPressureVariable
    solver_sys = pressure_system
    initial_condition = 0.2
    two_term_boundary_expansion = false
  []
  [T_fluid]
    type = INSFVEnergyVariable
    solver_sys = energy_system
    two_term_boundary_expansion = false
  []
[]

[FVKernels]
  inactive = 'u_buoyancy u_gravity v_buoyancy v_gravity'
  [u_advection]
    type = INSFVMomentumAdvection
    variable = vel_x
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'x'
  []
  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_x
    mu = ${mu}
    momentum_component = 'x'
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = vel_x
    momentum_component = 'x'
    pressure = pressure
    extra_vector_tags = ${pressure_tag}
  []
  [u_buoyancy]
    type = INSFVMomentumBoussinesq
    variable = vel_x
    T_fluid = T_fluid
    gravity = '0 -1 0'
    rho = ${rho}
    ref_temperature = ${temp_ref}
    alpha_name = ${alpha}
    momentum_component = 'x'
  []
  [u_gravity]
    type = INSFVMomentumGravity
    variable = vel_x
    gravity = '0 -1 0'
    rho = ${rho}
    momentum_component = 'x'
  []

  [v_advection]
    type = INSFVMomentumAdvection
    variable = vel_y
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'y'
  []
  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_y
    mu = ${mu}
    momentum_component = 'y'
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = vel_y
    momentum_component = 'y'
    pressure = pressure
    extra_vector_tags = ${pressure_tag}
  []
  [v_buoyancy]
    type = INSFVMomentumBoussinesq
    variable = vel_y
    T_fluid = T_fluid
    gravity = '0 -1 0'
    rho = ${rho}
    ref_temperature = ${temp_ref}
    alpha_name = ${alpha}
    momentum_component = 'y'
  []
  [v_gravity]
    type = INSFVMomentumGravity
    variable = vel_y
    gravity = '0 -1 0'
    rho = ${rho}
    momentum_component = 'y'
  []

  [p_diffusion]
    type = FVAnisotropicDiffusion
    variable = pressure
    coeff = "Ainv"
    coeff_interp_method = 'average'
  []
  [p_source]
    type = FVDivergence
    variable = pressure
    vector_field = "HbyA"
    force_boundary_execution = true
  []

  [temp_conduction]
    type = FVDiffusion
    coeff = ${k}
    variable = T_fluid
  []

  [temp_advection]
    type = INSFVEnergyAdvection
    variable = T_fluid
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = ${advected_interp_method}
  []
[]

[FVBCs]
  [top_x]
    type = INSFVNoSlipWallBC
    variable = vel_x
    boundary = 'top'
    function = 1
  []
  [no_slip_x]
    type = INSFVNoSlipWallBC
    variable = vel_x
    boundary = 'left right bottom'
    function = 0
  []
  [no_slip_y]
    type = INSFVNoSlipWallBC
    variable = vel_y
    boundary = 'left right top bottom'
    function = 0
  []
  [zero-grad-pressure]
    type = FVFunctionNeumannBC
    variable = pressure
    boundary = 'left right top bottom'
    function = 0.0
  []
  [T_hot]
    type = FVDirichletBC
    variable = T_fluid
    boundary = 'bottom'
    value = 1
  []

  [T_cold]
    type = FVDirichletBC
    variable = T_fluid
    boundary = 'top'
    value = 0
  []
[]

[FunctorMaterials]
  [ins_fv]
    type = INSFVEnthalpyFunctorMaterial
    temperature = 'T_fluid'
    rho = ${rho}
    cp = ${cp}
  []
[]

[Executioner]
  type = SIMPLENonlinearAssembly
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  energy_system = 'energy_system'
  pressure_gradient_tag = ${pressure_tag}
  momentum_equation_relaxation = 0.90
  energy_equation_relaxation = 0.99
  pressure_variable_relaxation = 0.30
  num_iterations = 150
  pressure_absolute_tolerance = 1e-13
  momentum_absolute_tolerance = 1e-13
  energy_absolute_tolerance = 1e-13
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  energy_petsc_options_value = 'hypre boomeramg'

  momentum_l_abs_tol = 1e-14
  energy_l_abs_tol = 1e-14
  pressure_l_abs_tol = 1e-14
  momentum_l_max_its = 30
  pressure_l_max_its = 30
  momentum_l_tol = 0.0
  energy_l_tol = 0.0
  pressure_l_tol = 0.0
  print_fields = false

  pin_pressure = true
  pressure_pin_value = 0.0
  pressure_pin_point = '0.01 0.099 0.0'
[]

[Outputs]
  exodus = true
  csv = false
  perf_graph = false
  print_nonlinear_residuals = false
  print_linear_residuals = true
[]
