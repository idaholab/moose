porosity_low = 0.35
porosity_high = 0.8
mu = 5e-3
rho = 1.0

[Mesh]
  [base]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 0
    xmax = 1
    nx = 8
  []
  [low_block]
    type = SubdomainBoundingBoxGenerator
    input = base
    block_id = 1
    xmin = 0
    xmax = 0.5
  []
[]

[Functions]
  [eps_fun]
    type = ParsedFunction
    expression = '(x < 0.5) ? ${porosity_low} : ${porosity_high}'
  []
  [eps_mu_fun]
    type = ParsedFunction
    expression = '(x < 0.5) ? ${porosity_low} * ${mu} : ${porosity_high} * ${mu}'
  []
[]

[UserObjects]
  [pns_rc]
    type = PorousRhieChowMassFlux
    u = vel_x
    pressure = pressure
    rho = ${rho}
    porosity = eps_fun
    p_diffusion_kernel = p_diffusion
  []
[]

[Variables]
  [vel_x]
    type = MooseLinearVariableFVReal
    initial_condition = 0.1
    solver_sys = u_system
  []
  [pressure]
    type = MooseLinearVariableFVReal
    initial_condition = 0.0
    solver_sys = pressure_system
  []
[]

[LinearFVKernels]
  [momentum_flux]
    type = LinearWCNSFVMomentumFlux
    variable = vel_x
    advected_interp_method = average
    mu = eps_mu_fun
    u = vel_x
    momentum_component = 'x'
    rhie_chow_user_object = pns_rc
    use_nonorthogonal_correction = false
  []
  [momentum_pressure]
    type = LinearPNSFVMomentumPressure
    variable = vel_x
    pressure = pressure
    porosity = eps_fun
    momentum_component = 'x'
  []
  [p_diffusion]
    type = LinearFVAnisotropicDiffusion
    variable = pressure
    diffusion_tensor = Ainv
    use_nonorthogonal_correction = false
  []
  [hbya_div]
    type = LinearFVDivergence
    variable = pressure
    face_flux = HbyA
    force_boundary_execution = true
  []
[]

[LinearFVBCs]
  [inlet_u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = vel_x
    boundary = left
    functor = 0.2
  []
  [outlet_u]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = vel_x
    boundary = right
    use_two_term_expansion = false
  []
  [outlet_p]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = pressure
    boundary = right
    functor = 0.0
  []
[]

[Problem]
  linear_sys_names = 'u_system pressure_system'
  previous_nl_solution_required = true
[]

[Executioner]
  type = SIMPLE
  rhie_chow_user_object = pns_rc
  momentum_systems = 'u_system'
  pressure_system = pressure_system
  num_iterations = 40
  momentum_l_abs_tol = 1e-12
  pressure_l_abs_tol = 1e-12
  pressure_variable_relaxation = 0.3
  momentum_equation_relaxation = 0.7
  momentum_petsc_options_iname = '-pc_type'
  momentum_petsc_options_value = 'hypre'
  pressure_petsc_options_iname = '-pc_type'
  pressure_petsc_options_value = 'hypre'
[]

[Preconditioning]
  active = ''
[]

[Postprocessors]
  [avg_low]
    type = ElementAverageValue
    variable = vel_x
    block = '1'
  []
  [avg_high]
    type = ElementAverageValue
    variable = vel_x
    block = '0'
  []
  [scaled_difference]
    type = ParsedPostprocessor
    value = '${porosity_low} * avg_low - ${porosity_high} * avg_high'
    pp_names = 'avg_low avg_high'
  []
[]

[Outputs]
  exodus = true
  file_base = porous_linear_basic_out
  csv = true
[]
