mu = 1e-2
rho = 1
gamma = 1e4
degree = 2
alpha = '${fparse 10 * degree^2}'

[Mesh]
  [file]
    type = FileMeshGenerator
    file = coarse06.msh
  []
  second_order = true
[]

[Problem]
  type = NavierStokesProblem
  extra_tag_matrices = 'mass'
  mass_matrix = 'mass'
  use_pressure_mass_matrix = true
[]

[Variables]
  [vel_x]
    family = L2_LAGRANGE
    order = SECOND
  []
  [vel_y]
    family = L2_LAGRANGE
    order = SECOND
  []
  [pressure]
    family = L2_LAGRANGE
    order = FIRST
  []
  [vel_bar_x]
    family = LAGRANGE
    order = SECOND
  []
  [vel_bar_y]
    family = LAGRANGE
    order = SECOND
  []
  [pressure_bar]
    family = LAGRANGE
    order = SECOND
  []
[]

[HDGKernels]
  [momentum_x_advection]
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
    diffusivity = 'mu'
    alpha = ${alpha}
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    component = 0
  []
  [momentum_y_advection]
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
    diffusivity = 'mu'
    alpha = ${alpha}
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    component = 1
  []
  [pressure]
    type = MassContinuityIPHDGKernel
    variable = pressure
    face_variable = pressure_bar
    interior_velocity_vars = 'vel_x vel_y'
    face_velocity_functors = 'vel_bar_x vel_bar_y'
  []

  [u_jump]
    type = MassFluxPenaltyIPHDG
    variable = vel_x
    face_variable = vel_bar_x
    face_velocity = face_velocity
    u = vel_x
    v = vel_y
    component = 0
    gamma = ${gamma}
  []
  [v_jump]
    type = MassFluxPenaltyIPHDG
    variable = vel_y
    face_variable = vel_bar_y
    face_velocity = face_velocity
    u = vel_x
    v = vel_y
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
  #
  # inlet
  #
  [momentum_x_advection_inlet]
    type = AdvectionIPHDGDirichletBC
    boundary = '1'
    face_variable = vel_bar_x
    functor = u_inlet
    variable = vel_x
    velocity = velocity
    coeff = ${rho}
  []
  [momentum_x_diffusion_inlet]
    type = NavierStokesStressIPHDGDirichletBC
    boundary = '1'
    variable = vel_x
    face_variable = vel_bar_x
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    alpha = ${alpha}
    functor = 'u_inlet'
    diffusivity = 'mu'
    component = 0
  []
  [momentum_y_diffusion_inlet]
    type = NavierStokesStressIPHDGDirichletBC
    boundary = '1'
    variable = vel_y
    face_variable = vel_bar_y
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    alpha = ${alpha}
    functor = 0
    diffusivity = 'mu'
    component = 1
  []
  [mass_inlet]
    type = MassContinuityIPHDGBC
    face_variable = pressure_bar
    variable = pressure
    boundary = '1'
    face_velocity_functors = 'u_inlet 0'
    interior_velocity_vars = 'vel_x vel_y'
  []

  #
  # walls
  #
  [momentum_x_diffusion_walls]
    type = NavierStokesStressIPHDGDirichletBC
    boundary = '2'
    variable = vel_x
    face_variable = vel_bar_x
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    alpha = ${alpha}
    functor = '0'
    diffusivity = 'mu'
    component = 0
  []
  [momentum_y_diffusion_walls]
    type = NavierStokesStressIPHDGDirichletBC
    boundary = '2'
    variable = vel_y
    face_variable = vel_bar_y
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    alpha = ${alpha}
    functor = 0
    diffusivity = 'mu'
    component = 1
  []
  [mass_walls]
    type = MassContinuityIPHDGBC
    face_variable = pressure_bar
    variable = pressure
    boundary = '2'
    face_velocity_functors = '0 0'
    interior_velocity_vars = 'vel_x vel_y'
  []

  #
  # Neumann
  #
  [momentum_x_advection_neumann]
    type = AdvectionIPHDGOutflowBC
    boundary = '3'
    constrain_lm = false
    face_variable = vel_bar_x
    variable = vel_x
    velocity = velocity
    coeff = ${rho}
  []
  [momentum_y_advection_neumann]
    type = AdvectionIPHDGOutflowBC
    boundary = '3'
    constrain_lm = false
    face_variable = vel_bar_y
    variable = vel_y
    velocity = velocity
    coeff = ${rho}
  []
  [momentum_x_diffusion_neumann]
    type = NavierStokesStressIPHDGPrescribedFluxBC
    boundary = '3'
    component = 0
    diffusivity = 'mu'
    face_variable = vel_bar_x
    prescribed_normal_flux = 0
    pressure_face_variable = pressure_bar
    pressure_variable = pressure
    variable = vel_x
    alpha = ${alpha}
  []
  [momentum_y_diffusion_neumann]
    type = NavierStokesStressIPHDGPrescribedFluxBC
    boundary = '3'
    component = 1
    diffusivity = 'mu'
    face_variable = vel_bar_y
    prescribed_normal_flux = 0
    pressure_face_variable = pressure_bar
    pressure_variable = pressure
    variable = vel_y
    alpha = ${alpha}
  []
  [mass_neumann]
    type = MassContinuityIPHDGBC
    face_variable = pressure_bar
    variable = pressure
    boundary = '3'
    face_velocity_functors = 'vel_bar_x vel_bar_y'
    interior_velocity_vars = 'vel_x vel_y'
  []

  [pb_mass]
    type = MassMatrixIntegratedBC
    variable = pressure_bar
    matrix_tags = 'mass'
    boundary = '1 2 3'
    density = '${fparse -1/gamma}'
  []

  [u_jump_walls]
    type = MassFluxPenaltyBC
    variable = vel_x
    face_variable = vel_bar_x
    u = vel_x
    v = vel_y
    component = 0
    boundary = '2'
    gamma = ${gamma}
    face_velocity = vel_walls
    dirichlet_boundary = true
  []
  [v_jump_walls]
    type = MassFluxPenaltyBC
    variable = vel_y
    face_variable = vel_bar_y
    u = vel_x
    v = vel_y
    component = 1
    boundary = '2'
    gamma = ${gamma}
    face_velocity = vel_walls
    dirichlet_boundary = true
  []
  [u_jump_inlet]
    type = MassFluxPenaltyBC
    variable = vel_x
    face_variable = vel_bar_x
    u = vel_x
    v = vel_y
    component = 0
    boundary = '1'
    gamma = ${gamma}
    face_velocity = vel_inlet
    dirichlet_boundary = true
  []
  [v_jump_inlet]
    type = MassFluxPenaltyBC
    variable = vel_y
    face_variable = vel_bar_y
    u = vel_x
    v = vel_y
    component = 1
    boundary = '1'
    gamma = ${gamma}
    face_velocity = vel_inlet
    dirichlet_boundary = true
  []
  [u_jump_outlet]
    type = MassFluxPenaltyBC
    variable = vel_x
    face_variable = vel_bar_x
    u = vel_x
    v = vel_y
    component = 0
    boundary = '3'
    gamma = ${gamma}
    face_velocity = face_velocity
    dirichlet_boundary = false
  []
  [v_jump_outlet]
    type = MassFluxPenaltyBC
    variable = vel_y
    face_variable = vel_bar_y
    u = vel_x
    v = vel_y
    component = 1
    boundary = '3'
    gamma = ${gamma}
    face_velocity = face_velocity
    dirichlet_boundary = false
  []
[]

[Materials]
  [const]
    type = ADGenericConstantMaterial
    prop_names = 'rho mu'
    prop_values = '${rho} ${mu}'
  []
  [vel]
    type = ADVectorFromComponentVariablesMaterial
    vector_prop_name = 'velocity'
    u = vel_x
    v = vel_y
  []
[]

[Functions]
  [u_inlet]
    type = ParsedFunction
    expression = '4*(2-y)*(y-1)'
  []
[]

[FunctorMaterials]
  [face_velocity]
    type = ADGenericVectorFunctorMaterial
    prop_names = face_velocity
    prop_values = 'vel_bar_x vel_bar_y 0'
  []
  [vel_inlet]
    type = GenericVectorFunctorMaterial
    prop_names = vel_inlet
    prop_values = 'u_inlet 0 0'
  []
  [vel_walls]
    type = GenericVectorFunctorMaterial
    prop_names = vel_walls
    prop_values = '0 0 0'
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
      petsc_options_value = 'full                            self                              300                fgmres    right        1e-4      300         1e-9'
    []
    [u]
      vars = 'vel_bar_x vel_bar_y'
      # petsc_options = '-ksp_converged_reason'
      # petsc_options_iname = '-pc_type -ksp_type -ksp_rtol -ksp_gmres_restart -ksp_pc_side -pc_factor_mat_solver_type -ksp_max_it -ksp_atol -ksp_norm_type'
      # petsc_options_value = 'ilu      gmres     1e-2      300                right        strumpack                  30          1e-8      unpreconditioned'
      petsc_options_iname = '-pc_type -ksp_type -pc_factor_mat_solver_type'
      petsc_options_value = 'ilu      preonly   strumpack'
    []
    [p]
      vars = 'pressure_bar'
      # petsc_options = '-ksp_converged_reason'
      # petsc_options_iname = '-pc_type -ksp_type -ksp_rtol -ksp_gmres_restart -ksp_pc_side -pc_factor_mat_solver_type -ksp_max_it -ksp_atol -ksp_norm_type'
      # petsc_options_value = 'ilu      gmres     1e-2      300                right        strumpack                  30          1e-8      unpreconditioned'
      petsc_options_iname = '-pc_type -pc_jacobi_type -ksp_type'
      petsc_options_value = 'jacobi   rowsum          preonly'
    []
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-ksp_type'
  petsc_options_value = 'preonly'
  nl_rel_tol = 1e-12
[]

[Outputs]
  print_linear_residuals = false
  [out]
    type = Exodus
    hide = 'pressure_bar vel_bar_x vel_bar_y'
  []
[]
