H = 1 #halfwidth of the channel
L = 100

Re = 13700

rho = 1
bulk_u = 1
mu = '${fparse rho * bulk_u * 2 * H / Re}'

advected_interp_method = 'upwind'

### k-epsilon Closure Parameters ###
sigma_k = 1.0
sigma_eps = 1.3
C1_eps = 1.44
C2_eps = 1.92
C_mu = 0.09

### Initial and Boundary Conditions ###
intensity = ${fparse 0.16*Re^(-1./8.)}
k_init = '${fparse 1.5*(intensity * bulk_u)^2}'
eps_init = '${fparse C_mu^0.75 * k_init^1.5 / (2*H)}'

### Modeling parameters ###
bulk_wall_treatment = false
walls = 'top bottom'
wall_treatment = 'eq_newton'  # Options: eq_newton, eq_incremental, eq_linearized, neq

[Mesh]
  [block_1]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = ${L}
    ymin = 0
    ymax = ${H}
    nx = 4
    ny = 4
    bias_y = 0.7
  []
  [block_2]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = ${L}
    ymin = ${fparse -H}
    ymax = 0
    nx = 4
    ny = 4
    bias_y = ${fparse 1/0.7}
  []
  [smg]
    type = StitchedMeshGenerator
    inputs = 'block_1 block_2'
    clear_stitched_boundary_ids = true
    stitch_boundaries_pairs = 'bottom top'
    merge_boundaries_with_same_name = true
  []
  # Prevent test diffing on distributed parallel element numbering
  allow_renumbering = false
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system TKE_system TKED_system'
  previous_nl_solution_required = true
[]

[GlobalParams]
  rhie_chow_user_object = 'rc'
  advected_interp_method = ${advected_interp_method}
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
    initial_condition = ${bulk_u}
    solver_sys = u_system
  []
  [vel_y]
    type = MooseLinearVariableFVReal
    initial_condition = 0
    solver_sys = v_system
  []
  [pressure]
    type = MooseLinearVariableFVReal
    initial_condition = 1e-8
    solver_sys = pressure_system
  []
  [TKE]
    type = MooseLinearVariableFVReal
    solver_sys = TKE_system
    initial_condition = ${k_init}
  []
  [TKED]
    type = MooseLinearVariableFVReal
    solver_sys = TKED_system
    initial_condition = ${eps_init}
  []
[]

[LinearFVKernels]
  [u_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_x
    advected_interp_method = ${advected_interp_method}
    mu = 'mu_t'
    u = vel_x
    v = vel_y
    momentum_component = 'x'
    rhie_chow_user_object = 'rc'
    use_nonorthogonal_correction = false
    use_deviatoric_terms = yes
  []
  [u_diffusion]
    type = LinearFVDiffusion
    variable = vel_x
    diffusion_coeff = ${mu}
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
    advected_interp_method = ${advected_interp_method}
    mu = 'mu_t'
    u = vel_x
    v = vel_y
    momentum_component = 'y'
    rhie_chow_user_object = 'rc'
    use_nonorthogonal_correction = false
    use_deviatoric_terms = yes
  []
  [v_diffusion]
    type = LinearFVDiffusion
    variable = vel_y
    diffusion_coeff = ${mu}
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

  [TKE_advection]
    type = LinearFVTurbulentAdvection
    variable = TKE
  []
  [TKE_diffusion]
    type = LinearFVTurbulentDiffusion
    variable = TKE
    diffusion_coeff = ${mu}
    use_nonorthogonal_correction = false
  []
  [TKE_turb_diffusion]
    type = LinearFVTurbulentDiffusion
    variable = TKE
    diffusion_coeff = 'mu_t'
    scaling_coeff = ${sigma_k}
    use_nonorthogonal_correction = false
  []
  [TKE_source_sink]
    type = LinearFVTKESourceSink
    variable = TKE
    u = vel_x
    v = vel_y
    epsilon = TKED
    rho = ${rho}
    mu = ${mu}
    mu_t = 'mu_t'
    walls = ${walls}
    wall_treatment = ${wall_treatment}
    C_pl = 1e10
  []

  [TKED_advection]
    type = LinearFVTurbulentAdvection
    variable = TKED
    walls = ${walls}
  []
  [TKED_diffusion]
    type = LinearFVTurbulentDiffusion
    variable = TKED
    diffusion_coeff = ${mu}
    use_nonorthogonal_correction = false
    walls = ${walls}
  []
  [TKED_turb_diffusion]
    type = LinearFVTurbulentDiffusion
    variable = TKED
    diffusion_coeff = 'mu_t'
    scaling_coeff = ${sigma_eps}
    use_nonorthogonal_correction = false
    walls = ${walls}
  []
  [TKED_source_sink]
    type = LinearFVTKEDSourceSink
    variable = TKED
    u = vel_x
    v = vel_y
    tke = TKE
    rho = ${rho}
    mu = ${mu}
    mu_t = 'mu_t'
    C1_eps = ${C1_eps}
    C2_eps = ${C2_eps}
    walls = ${walls}
    wall_treatment = ${wall_treatment}
    C_pl = 1e10
  []
[]

[LinearFVBCs]
  [inlet-u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left'
    variable = vel_x
    functor = '${bulk_u}'
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
  [outlet_u]
    type = LinearFVAdvectionDiffusionOutflowBC
    boundary = 'right'
    variable = vel_x
    use_two_term_expansion = false
  []
  [outlet_v]
    type = LinearFVAdvectionDiffusionOutflowBC
    boundary = 'right'
    variable = vel_y
    use_two_term_expansion = false
  []
  [outlet_p]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'right'
    variable = pressure
    functor = 0.0
  []

  [inlet_TKE]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left'
    variable = TKE
    functor = '${k_init}'
  []
  [outlet_TKE]
    type = LinearFVAdvectionDiffusionOutflowBC
    boundary = 'right'
    variable = TKE
    use_two_term_expansion = false
  []
  [inlet_TKED]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left'
    variable = TKED
    functor = '${eps_init}'
  []
  [outlet_TKED]
    type = LinearFVAdvectionDiffusionOutflowBC
    boundary = 'right'
    variable = TKED
    use_two_term_expansion = false
  []
  [walls_mu_t]
    type = LinearFVTurbulentViscosityWallFunctionBC
    boundary = 'bottom top'
    variable = 'mu_t'
    u = vel_x
    v = vel_y
    rho = ${rho}
    mu = ${mu}
    tke = TKE
    wall_treatment = ${wall_treatment}
  []
[]

[AuxVariables]
  [mu_t]
    type = MooseLinearVariableFVReal
    initial_condition = '${fparse rho * C_mu * ${k_init}^2 / eps_init}'
  []
  [yplus]
    type = MooseLinearVariableFVReal
  []
[]

[AuxKernels]
  [compute_mu_t]
    type = kEpsilonViscosityAux
    variable = mu_t
    C_mu = ${C_mu}
    tke = TKE
    epsilon = TKED
    mu = ${mu}
    rho = ${rho}
    u = vel_x
    v = vel_y
    bulk_wall_treatment = ${bulk_wall_treatment}
    walls = ${walls}
    wall_treatment = ${wall_treatment}
    execute_on = 'NONLINEAR'
    mu_t_ratio_max = 1e20
  []
  [compute_y_plus]
    type = RANSYPlusAux
    variable = yplus
    tke = TKE
    mu = ${mu}
    rho = ${rho}
    u = vel_x
    v = vel_y
    walls = ${walls}
    wall_treatment = ${wall_treatment}
    execute_on = 'NONLINEAR'
  []
[]

[Executioner]
  type = SIMPLE

  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  turbulence_systems = 'TKE_system TKED_system'

  momentum_l_abs_tol = 1e-14
  pressure_l_abs_tol = 1e-14
  turbulence_l_abs_tol = 1e-14
  momentum_l_tol = 1e-14
  pressure_l_tol = 1e-14
  turbulence_l_tol = 1e-14

  momentum_equation_relaxation = 0.7
  pressure_variable_relaxation = 0.3
  turbulence_equation_relaxation = '0.2 0.2'
  turbulence_field_relaxation = '0.2 0.2'
  num_iterations = 1000
  pressure_absolute_tolerance = 1e-12
  momentum_absolute_tolerance = 1e-12
  turbulence_absolute_tolerance = '1e-12 1e-12'
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  turbulence_petsc_options_iname = '-pc_type -pc_hypre_type'
  turbulence_petsc_options_value = 'hypre boomeramg'

  print_fields = false
  continue_on_max_its = true
[]

[Outputs]
  csv = true
[]

[VectorPostprocessors]
  [side_bottom]
    type = SideValueSampler
    boundary = 'bottom'
    variable = 'vel_x vel_y pressure TKE TKED'
    sort_by = 'x'
    execute_on = 'timestep_end'
  []
  [side_top]
    type = SideValueSampler
    boundary = 'top'
    variable = 'vel_x vel_y pressure TKE TKED'
    sort_by = 'x'
    execute_on = 'timestep_end'
  []
  [line_center_channel]
    type = LineValueSampler
    start_point = '${fparse 0.125 * L} ${fparse 0.0001} 0'
    end_point = '${fparse 0.875 * L} ${fparse 0.0001} 0'
    num_points = ${Mesh/block_1/nx}
    variable = 'vel_x vel_y pressure TKE TKED'
    sort_by = 'x'
    execute_on = 'timestep_end'
  []
  [line_quarter_radius_channel]
    type = LineValueSampler
    start_point = '${fparse 0.125 * L} ${fparse 0.5 * H} 0'
    end_point = '${fparse 0.875 * L} ${fparse 0.5 * H} 0'
    num_points =  ${Mesh/block_1/nx}
    variable = 'vel_x vel_y pressure TKE TKED'
    sort_by = 'x'
    execute_on = 'timestep_end'
  []
[]
