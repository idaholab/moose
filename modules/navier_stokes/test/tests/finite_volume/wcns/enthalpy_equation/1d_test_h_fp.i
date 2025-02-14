L = 30
nx = 600
bulk_u = 0.01
p_ref = 101325.0
T_in = 860.
q_source = 20000.
advected_interp_method = 'upwind'

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 0
    xmax = ${L}
    nx = ${nx}
  []
  allow_renumbering = false
[]

[GlobalParams]
  rhie_chow_user_object = 'rc'
  advected_interp_method = ${advected_interp_method}
  u = vel_x
[]

[Problem]
  linear_sys_names = 'u_system pressure_system energy_system'
  previous_nl_solution_required = true
[]

[UserObjects]
  [rc]
    type = RhieChowMassFlux
    u = vel_x
    pressure = pressure
    rho = 'rho'
    p_diffusion_kernel = p_diffusion
  []
[]

[Variables]
  [vel_x]
    type = MooseLinearVariableFVReal
    solver_sys = u_system
    initial_condition = ${bulk_u}
  []
  [pressure]
    type = MooseLinearVariableFVReal
    solver_sys = pressure_system
    initial_condition = ${p_ref}
  []
  [h]
    type = MooseLinearVariableFVReal
    solver_sys = energy_system
    initial_condition = ${fparse 860.*240.}
  []
[]

[AuxVariables]
  [rho_var]
    type = MooseLinearVariableFVReal
  []
  [cp_var]
    type = MooseLinearVariableFVReal
  []
  [mu_var]
    type = MooseLinearVariableFVReal
  []
  [k_var]
    type = MooseLinearVariableFVReal
  []
  [alpha_var]
    type = MooseLinearVariableFVReal
  []
  [T]
    type = MooseLinearVariableFVReal
    initial_condition = ${T_in}
  []
  [h_aux]
    type = MooseLinearVariableFVReal
  []
[]

[LinearFVKernels]

  [u_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_x
    mu = 'mu'
    momentum_component = 'x'
    use_nonorthogonal_correction = false
  []
  [u_pressure]
    type = LinearFVMomentumPressure
    variable = vel_x
    pressure = pressure
    momentum_component = 'x'
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

  [temp_advection]
    type = LinearFVEnergyAdvection
    variable = h
  []
  [source]
    type = LinearFVSource
    variable = h
    source_density = source_func
  []
[]

[LinearFVBCs]
  [inlet_u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left'
    variable = vel_x
    functor = ${bulk_u}
  []
  [inlet_h]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = h
    boundary = 'left'
    functor = 'h_from_p_T'
  []
  [inlet_T]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T
    boundary = 'left'
    functor = ${T_in}
  []

  [outlet_p]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'right'
    variable = pressure
    functor = ${p_ref}
  []
  [outlet_h]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = h
    use_two_term_expansion = false
    boundary = 'right'
  []
  [outlet_u]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = vel_x
    use_two_term_expansion = false
    boundary = 'right'
  []
[]

[FluidProperties]
  [lead]
    type = LeadFluidProperties
  []
[]

[FunctorMaterials]
  [enthalpy_material]
    type = LinearFVEnthalpyFunctorMaterial
    pressure = ${p_ref}
    T_fluid = T
    h = h
    fp = lead
  []
  [fluid_props_to_mat_props]
    type = GeneralFunctorFluidProps
    fp = lead
    pressure = ${p_ref}
    T_fluid = 'T'
    speed = 1
    porosity = 1
    characteristic_length = 1
  []
  [source_func]
    type = ADParsedFunctorMaterial
    property_name = source_func
    functor_names = 'rho'
    expression = ${q_source}
  []
[]

[AuxKernels]
  [rho_out]
    type = FunctorAux
    functor = 'rho'
    variable = 'rho_var'
    execute_on = 'NONLINEAR'
  []
  [cp_out]
    type = FunctorAux
    functor = 'cp'
    variable = 'cp_var'
    execute_on = 'NONLINEAR'
  []
  [mu_out]
    type = FunctorAux
    functor = 'mu'
    variable = 'mu_var'
    execute_on = 'NONLINEAR'
  []
  [k_out]
    type = FunctorAux
    functor = 'k'
    variable = 'k_var'
    execute_on = 'NONLINEAR'
  []
  [T_from_h_functor_aux]
    type = FunctorAux
    functor = 'T_from_p_h'
    variable = 'T'
    execute_on = 'NONLINEAR'
  []
  [h_from_T_functor_aux]
    type = FunctorAux
    functor = 'h_from_p_T'
    variable = 'h_aux'
    execute_on = 'NONLINEAR'
  []
[]

[Postprocessors]
  [T_out_sim]
    type = ElementalVariableValue
    variable = T
    elementid = ${fparse nx-1}
  []
[]

[Executioner]
  type = SIMPLE
  momentum_l_abs_tol = 1e-12
  pressure_l_abs_tol = 1e-12
  energy_l_abs_tol = 1e-12
  momentum_l_tol = 0
  pressure_l_tol = 0
  energy_l_tol = 0
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system'
  pressure_system = 'pressure_system'
  energy_system = 'energy_system'
  momentum_equation_relaxation = 0.7
  pressure_variable_relaxation = 0.3
  energy_equation_relaxation = 0.95
  num_iterations = 100
  pressure_absolute_tolerance = 1e-8
  momentum_absolute_tolerance = 1e-8
  energy_absolute_tolerance = 1e-6
  print_fields = false
  momentum_l_max_its = 200

  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'

  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'

  energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  energy_petsc_options_value = 'hypre boomeramg'
  continue_on_max_its = true
[]

[Outputs]
  [out]
    type = CSV
  []
[]
