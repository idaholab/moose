rho = 1
l = 2
U = 1
n = 16
gamma = 1e4
degree = 2
alpha = '${fparse 10 * degree^2}'

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
    elem_type = TRI6
  []
[]

[Problem]
  type = NavierStokesProblem
  extra_tag_matrices = 'mass'
  mass_matrix = 'mass'
  use_pressure_mass_matrix = true
[]

[Variables]
  [vel_x]
    family = L2_HIERARCHIC
    order = SECOND
  []
  [vel_y]
    family = L2_HIERARCHIC
    order = SECOND
  []
  [pressure]
    family = L2_HIERARCHIC
    order = FIRST
  []
  [vel_bar_x]
    family = SIDE_HIERARCHIC
    order = SECOND
  []
  [vel_bar_y]
    family = SIDE_HIERARCHIC
    order = SECOND
  []
  [pressure_bar]
    family = SIDE_HIERARCHIC
    order = SECOND
  []
[]

[Kernels]
  [utime]
    type = TimeDerivative
    variable = vel_x
  []
  [vtime]
    type = TimeDerivative
    variable = vel_y
  []
[]

[HDGKernels]
  [momentum_x_convection]
    type = AdvectionIPHDGKernel
    variable = vel_x
    face_variable = vel_bar_x
    velocity = 'velocity'
    coeff = ${rho}
  []
  [momentum_x_diffusion]
    type = NavierStokesStressIPHDGKernel
    variable = vel_x
    face_variable = vel_bar_x
    diffusivity = 'mu_t_traditional'
    alpha = ${alpha}
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    component = 0
  []
  [momentum_y_convection]
    type = AdvectionIPHDGKernel
    variable = vel_y
    face_variable = vel_bar_y
    velocity = 'velocity'
    coeff = ${rho}
  []
  [momentum_y_diffusion]
    type = NavierStokesStressIPHDGKernel
    variable = vel_y
    face_variable = vel_bar_y
    diffusivity = 'mu_t_traditional'
    alpha = ${alpha}
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    component = 1
  []
  [pressure_convection]
    type = AdvectionIPHDGKernel
    variable = pressure
    face_variable = pressure_bar
    velocity = 'velocity'
    coeff = '${fparse -rho}'
    self_advection = false
  []

  [u_jump]
    type = MassFluxPenaltyIPHDG
    variable = vel_x
    u = vel_x
    v = vel_y
    u_face = vel_bar_x
    v_face = vel_bar_y
    component = 0
    gamma = ${gamma}
  []
  [v_jump]
    type = MassFluxPenaltyIPHDG
    variable = vel_y
    u = vel_x
    v = vel_y
    u_face = vel_bar_x
    v_face = vel_bar_y
    component = 1
    gamma = ${gamma}
  []
  [pb_mass]
    type = MassMatrixHDG
    variable = pressure_bar
    matrix_tags = 'mass'
    density = '${fparse -1/gamma}'
  []
[]

[BCs]
  [momentum_x_diffusion_walls]
    type = NavierStokesStressIPHDGDirichletBC
    boundary = 'left bottom right'
    variable = vel_x
    face_variable = vel_bar_x
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    alpha = ${alpha}
    functor = '0'
    diffusivity = 'mu_t_traditional'
    component = 0
  []
  [momentum_x_diffusion_top]
    type = NavierStokesStressIPHDGDirichletBC
    boundary = 'top'
    variable = vel_x
    face_variable = vel_bar_x
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    alpha = ${alpha}
    functor = '${U}'
    diffusivity = 'mu_t_traditional'
    component = 0
  []
  [momentum_y_diffusion_all]
    type = NavierStokesStressIPHDGDirichletBC
    boundary = 'left bottom right top'
    variable = vel_y
    face_variable = vel_bar_y
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    alpha = ${alpha}
    functor = '0'
    diffusivity = 'mu_t_traditional'
    component = 1
  []

  [mass_convection]
    type = AdvectionIPHDGPrescribedFluxBC
    face_variable = pressure_bar
    variable = pressure
    velocity = 'velocity'
    coeff = '${fparse -rho}'
    self_advection = false
    boundary = 'left bottom top right'
  []

  [pb_mass]
    type = MassMatrixIntegratedBC
    variable = pressure_bar
    matrix_tags = 'mass'
    boundary = 'left right bottom top'
    density = '${fparse -1/gamma}'
  []

  [u_jump_walls]
    type = MassFluxPenaltyBC
    variable = vel_x
    u = vel_x
    v = vel_y
    component = 0
    boundary = 'left right bottom'
    gamma = ${gamma}
  []
  [v_jump_walls]
    type = MassFluxPenaltyBC
    variable = vel_y
    u = vel_x
    v = vel_y
    component = 1
    boundary = 'left right bottom'
    gamma = ${gamma}
  []
  [u_jump_top]
    type = MassFluxPenaltyBC
    variable = vel_x
    u = vel_x
    v = vel_y
    component = 0
    boundary = 'top'
    gamma = ${gamma}
    dirichlet_value = top_vel
  []
  [v_jump_top]
    type = MassFluxPenaltyBC
    variable = vel_y
    u = vel_x
    v = vel_y
    component = 1
    boundary = 'top'
    gamma = ${gamma}
    dirichlet_value = top_vel
  []
[]

[Functions]
  [top_vel]
    type = ParsedVectorFunction
    expression_x = ${U}
  []
  [reynolds]
    type = ParsedFunction
    expression = '100 * t'
  []
[]

[AuxVariables]
  [mixing_len]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [mixing_len]
    type = WallDistanceMixingLengthAux
    variable = mixing_len
    walls = 'left right top bottom'
    execute_on = 'initial'
  []
[]

[FunctorMaterials]
  [mu]
    type = ADParsedFunctorMaterial
    expression = '${U} * ${l} / reynolds'
    property_name = mu
    functor_names = 'reynolds'
  []
  [total_viscosity]
    type = MixingLengthTurbulentViscosityFunctorMaterial
    u = vel_x
    v = vel_y
    mixing_length = mixing_len
    mu = mu
    rho = ${rho}
  []
[]

[Materials]
  [const]
    type = ADGenericConstantMaterial
    prop_names = 'rho'
    prop_values = '${rho}'
  []
  [vel]
    type = ADVectorFromComponentVariablesMaterial
    vector_prop_name = 'velocity'
    u = vel_x
    v = vel_y
  []
  [mu_t_traditional]
    type = MaterialFunctorConverter
    ad_props_out = 'mu_t_traditional'
    functors_in = 'total_viscosity'
  []
[]

[Preconditioning]
  [FSP]
    type = SCFSP
    topsplit = 'up'
    [up]
      splitting = 'u p'
      splitting_type = schur
      petsc_options = '-ksp_monitor'
      petsc_options_iname = '-pc_fieldsplit_schur_fact_type  -pc_fieldsplit_schur_precondition -ksp_gmres_restart -ksp_type -ksp_pc_side -ksp_rtol -ksp_max_it -ksp_atol'
      petsc_options_value = 'full                            self                              300                fgmres    right        1e-4      30          1e-9'
    []
    [u]
      vars = 'vel_bar_x vel_bar_y'
      petsc_options = '-ksp_monitor'
      petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart -pc_hypre_boomeramg_strong_threshold -pc_hypre_boomeramg_interp_type -pc_hypre_boomeramg_coarsen_type -pc_hypre_boomeramg_agg_nl -pc_hypre_boomeramg_agg_num_paths -pc_hypre_boomeramg_truncfactor'
      petsc_options_value = 'hypre boomeramg 301 0.25 ext+i PMIS 4 2 0.4'
    []
    [p]
      vars = 'pressure_bar'
      petsc_options = '-ksp_monitor'
      petsc_options_iname = '-pc_type -ksp_type -ksp_rtol -ksp_gmres_restart -ksp_pc_side -pc_factor_mat_solver_type -ksp_max_it'
      petsc_options_value = 'ilu      gmres     1e-2      300                right        strumpack                  30'
    []
  []
[]

[Executioner]
  type = Transient
  end_time = 1e3
  nl_abs_tol = 1e-7
  nl_rel_tol = 1e-8
  petsc_options_iname = '-snes_linesearch_type'
  petsc_options_value = 'basic'
  nl_max_its = 10
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1
    optimal_iterations = 6
    growth_factor = 1.5
  []
[]

[Outputs]
  print_linear_residuals = 'false'
  checkpoint = true
  [out]
    type = Exodus
    hide = 'pressure_average'
    output_material_properties = true
  []
[]

[Postprocessors]
  [Re]
    type = FunctionValuePostprocessor
    function = 'reynolds'
  []
  [pressure_average]
    type = ElementAverageValue
    variable = pressure
  []
[]

[Correctors]
  [set_pressure]
    type = NSPressurePin
    pin_type = 'average'
    variable = pressure
    pressure_average = 'pressure_average'
  []
[]
