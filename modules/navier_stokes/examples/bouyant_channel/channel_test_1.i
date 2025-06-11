mu = 1.0
rho = 1.0
k = 5.0
cp = 4816
alpha = 150
alpha_b = 0.001
Pr_t = 0.9
advected_interp_method = 'average'
velocity_interp_method = 'rc'

pressure_tag = "pressure_grad"

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '10.0'
    dy = '1.0'
    ix = '50'
    iy = '10'
  []
[]

[GlobalParams]
  rhie_chow_user_object = 'rc'
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
    initial_condition = 0.5
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
    initial_condition = 300
    solver_sys = energy_system
    two_term_boundary_expansion = false
  []
[]

[FVKernels]
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
  [u_viscosity_turbulent]
    type = INSFVMomentumDiffusion
    variable = vel_x
    mu = 'mu_t_torch'
    momentum_component = 'x'
    complete_expansion = true
    u = vel_x
    v = vel_y
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = vel_x
    momentum_component = 'x'
    pressure = pressure
    extra_vector_tags = ${pressure_tag}
  []
  [u_gravity]
    type = INSFVMomentumGravity
    momentum_component = 'x'
    variable = vel_x
    gravity = '0 -9.81 0'
    rho = ${rho}
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
  [v_viscosity_turbulent]
    type = INSFVMomentumDiffusion
    variable = vel_y
    mu = 'mu_t_torch'
    momentum_component = 'y'
    complete_expansion = true
    u = vel_x
    v = vel_y
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
    momentum_component = 'y'
    T_fluid = T_fluid
    gravity = '0 -9.81 0'
    rho = ${rho}
    ref_temperature = 300.0
  []
  [v_gravity]
    type = INSFVMomentumGravity
    variable = vel_y
    momentum_component = 'y'
    gravity = '0 -9.81 0'
    rho = ${rho}
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

  [energy_advection]
    type = INSFVEnergyAdvection
    variable = T_fluid
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = ${advected_interp_method}
  []
  [energy_diffusion]
    type = FVDiffusion
    coeff = ${k}
    variable = T_fluid
  []
  [temp_turb_conduction]
    type = FVDiffusion
    coeff = 'k_t'
    variable = T_fluid
  []
[]

[FVBCs]
  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = vel_x
    function = '1.0'
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = vel_y
    function = '0.0'
  []
  [walls-u]
    type = INSFVNoSlipWallBC
    boundary = 'top bottom'
    variable = vel_x
    function = 0.0
  []
  [walls-v]
    type = INSFVNoSlipWallBC
    boundary = 'top bottom'
    variable = vel_y
    function = 0.0
  []
  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'right'
    variable = pressure
    function = 1.0
  []
  [zero-grad-pressure]
    type = FVFunctionNeumannBC
    variable = pressure
    boundary = 'top left bottom'
    function = 0.0
  []
  [inlet_t]
    type = FVDirichletBC
    boundary = 'left'
    variable = T_fluid
    value = 300
  []
  [top_t]
    type = FVDirichletBC
    boundary = 'top'
    variable = T_fluid
    value = 400
  []
  [bottom_t]
    type = FVDirichletBC
    boundary = 'bottom'
    variable = T_fluid
    value = 300
  []
  ##########################################################
[]

[Executioner]
  type = SIMPLENonlinearAssembly
  momentum_l_abs_tol = 1e-11
  pressure_l_abs_tol = 1e-11
  energy_l_abs_tol = 1e-11
  momentum_l_tol = 0
  pressure_l_tol = 0
  energy_l_tol = 0
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  energy_system = 'energy_system'
  pressure_gradient_tag = ${pressure_tag}
  momentum_equation_relaxation = 0.7
  pressure_variable_relaxation = 0.3
  energy_equation_relaxation = 0.5
  num_iterations = 500
  pressure_absolute_tolerance = 1e-10
  momentum_absolute_tolerance = 1e-10
  energy_absolute_tolerance = 1e-10
  print_fields = false
  continue_on_max_its = true
[]

[UserObjects]
  [cody_net]
    type = TorchScriptUserObject
    filename = "cody_net.pt"
    load_during_construction = true
    execute_on = INITIAL
  []
[]



[Materials]
  [const_functor]
    type = ADGenericFunctorMaterial
    prop_names = 'cp alpha alpha_b'
    prop_values = '${cp} ${alpha} ${alpha_b}'
  []
  [const_mu_t]
    type = ADGenericFunctorMaterial
    prop_names = 'mu_t'
    prop_values = '0.1'
  []
  [net_material] # Populates mu_t_torch
    type = TorchScriptTurbulentViscosityMaterial
    torch_script_userobject = cody_net
    u = vel_x
    v = vel_y
  []
  [k_t]
    type = ADParsedFunctorMaterial
    expression = 'mu_t * cp / Pr_t'
    functor_names = 'mu_t ${cp} ${Pr_t}'
    functor_symbols = 'mu_t cp Pr_t'
    property_name = 'k_t'
  []
  [ins_fv]
    type = INSFVEnthalpyFunctorMaterial
    rho = ${rho}
    temperature = 'T_fluid'
  []
[]

[Outputs]
  exodus = true
  csv = false
  perf_graph = false
  print_nonlinear_residuals = false
  print_linear_residuals = true
[]
