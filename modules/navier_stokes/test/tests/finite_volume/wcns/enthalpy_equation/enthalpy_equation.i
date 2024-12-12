H = 0.05 #halfwidth of the channel, 10 cm of channel height
L = 30
bulk_u = 0.01
p_ref = 101325.0
#inlet_temp = 860.
#flux = 4000.0

advected_interp_method = 'upwind'

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = ${L}
    ymin = -${H}
    ymax = ${H}
    nx = 200
    ny = 80
    #bias_y = 0.95
  []
[]

[GlobalParams]
  rhie_chow_user_object = 'rc'
  advected_interp_method = ${advected_interp_method}
  #velocity_interp_method = 'rc'
  u = vel_x
  v = vel_y
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
  [vel_y]
    type = MooseLinearVariableFVReal
    solver_sys = v_system
    initial_condition = 0
  []
  [pressure]
    type = MooseLinearVariableFVReal
    solver_sys = pressure_system
    initial_condition = ${p_ref}
  []
  [h]
    type = MooseLinearVariableFVReal
    solver_sys = energy_system
    initial_condition = ${fparse 860.*1900.} # 1900 is an approx of cp(T)
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
  [T]
    type = MooseLinearVariableFVReal
    initial_condition = 860.
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

  [v_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_y
    mu = 'mu'
    momentum_component = 'y'
    use_nonorthogonal_correction = false
  []
  [v_pressure]
    type = LinearFVMomentumPressure
    variable = vel_y
    pressure = pressure
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

  [temp_conduction]
    type = LinearFVDiffusion
    diffusion_coeff = 'alpha'
    variable = h
  []
  [temp_advection]
    type = LinearFVEnergyAdvection
    variable = h
  []
[]

[LinearFVBCs]
  [inlet_u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left'
    variable = vel_x
    functor = ${bulk_u} #${bulk_u} #'fully_developed_velocity'
  []
  [inlet-v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left'
    variable = vel_y
    functor = 0
  []
  [inlet_h]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = h
    boundary = 'left'
    functor = ${fparse 1900.*860.}
  []
  # [inlet_T]
  #   type = LinearFVAdvectionDiffusionFunctorDirichletBC
  #   variable = T
  #   boundary = 'left'
  #   functor = 860.
  # []

  [walls-u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = vel_x
    boundary = 'top bottom'
    functor = 0.
  []
  [walls-v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = vel_y
    boundary = 'top bottom'
    functor = 0.
  []
  [walls_h]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = h
    boundary = 'top bottom'
    functor = ${fparse 1900. * 950}
  []
  # [walls_T]
  #   type = LinearFVAdvectionDiffusionFunctorDirichletBC
  #   variable = T
  #   boundary = 'top bottom'
  #   functor = 950.
  # []
  [walls_p]
    type = LinearFVExtrapolatedPressureBC
    boundary = 'top bottom'
    variable = pressure
    use_two_term_expansion = false
  []

  [outlet_p]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'right'
    variable = pressure
    functor = ${p_ref}
  []
  # [outlet_T]
  #   type = LinearFVAdvectionDiffusionOutflowBC
  #   variable = T
  #   use_two_term_expansion = false
  #   boundary = 'right'
  # []
  [outlet_h]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = h
    use_two_term_expansion = false
    boundary = 'right'
  []
[]

[UserObjects]
  [read_recycling]
    type = PropertyReadFile
    prop_file_name = 'LaminarInletProfile.csv'
    read_type = 'voronoi'
    nprop = 9
    execute_on = TIMESTEP_BEGIN
    nvoronoi = 30
  []
[]

[Functions]
  [fully_developed_velocity]
    type = PiecewiseConstantFromCSV
    read_prop_user_object = 'read_recycling'
    read_type = 'voronoi'
    column_number = '6'
  []
[]

[FluidProperties]
  [salt]
    type = SalineMoltenSaltFluidProperties
    comp_name = "LiF NaF KF"
    comp_val = "0.465 0.115 0.42"
    prop_def_file = "saline_custom.prp"
  []
[]

[FunctorMaterials]
  [fluid_props_to_mat_props]
    type = GeneralFunctorFluidProps
    fp = salt
    pressure = ${p_ref}
    T_fluid = 'T'
    speed = 1
    porosity = 1
    characteristic_length = 1
  []
  [alpha]
    type = ADParsedFunctorMaterial
    property_name = 'alpha'
    functor_names = 'k cp'
    expression = 'k/cp'
  []
  [enthalpy_material]
    type = LinearFVEnthalpyFunctorMaterial
    pressure = ${p_ref}
    T_fluid = T
    h = h
    fp = salt
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
  [T_from_h_functor]
    type = FunctorAux
    functor = 'T_from_p_h'
    variable = 'T'
    execute_on = 'NONLINEAR'
  []
[]

[Executioner]
  type = SIMPLE
  momentum_l_abs_tol = 1e-6
  pressure_l_abs_tol = 1e-6
  energy_l_abs_tol = 1e-6
  momentum_l_tol = 0
  pressure_l_tol = 0
  energy_l_tol = 0
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  energy_system = 'energy_system'
  momentum_equation_relaxation = 0.7
  pressure_variable_relaxation = 0.3
  energy_equation_relaxation = 0.8
  num_iterations = 1
  pressure_absolute_tolerance = 1e-8
  momentum_absolute_tolerance = 1e-8
  energy_absolute_tolerance = 1e-6
  print_fields = false
  momentum_l_max_its = 1000

  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'

  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'

  energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  energy_petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
  execute_on = 'TIMESTEP_BEGIN NONLINEAR'

[]
