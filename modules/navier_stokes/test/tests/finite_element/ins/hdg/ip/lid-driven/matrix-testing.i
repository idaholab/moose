mu = 1
rho = 1
U = 1
n = 5
l = 1

[GlobalParams]
  gamma = 1
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
    elem_type = TRI6
  []
[]

[Problem]
  type = PrintMatricesNSProblem
  extra_tag_matrices = 'mass jump combined grad_div'
  pressure_mass_matrix = 'mass'
  velocity_mass_matrix = 'mass'
  augmented_lagrange_matrices = 'jump grad_div combined'
  u = vel_x
  v = vel_y
  pressure = pressure
  pressure_bar = pressure_bar
  print = false
[]

[Variables]
  [vel_x]
    family = MONOMIAL
    order = FIRST
  []
  [vel_y]
    family = MONOMIAL
    order = FIRST
  []
  [pressure]
    family = MONOMIAL
    order = CONSTANT
  []
  [vel_bar_x]
    family = SIDE_HIERARCHIC
    order = FIRST
  []
  [vel_bar_y]
    family = SIDE_HIERARCHIC
    order = FIRST
  []
  [pressure_bar]
    family = SIDE_HIERARCHIC
    order = FIRST
  []
  [lambda]
    family = SCALAR
    order = FIRST
  []
[]

[HDGKernels]
  [momentum_x_diffusion]
    type = NavierStokesStressIPHDGKernel
    variable = vel_x
    face_variable = vel_bar_x
    diffusivity = 'mu'
    alpha = 6
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    component = 0
  []

  [momentum_y_diffusion]
    type = NavierStokesStressIPHDGKernel
    variable = vel_y
    face_variable = vel_bar_y
    diffusivity = 'mu'
    alpha = 6
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

  [pb_mass]
    type = MassMatrixHDG
    variable = pressure_bar
    matrix_tags = 'mass'
  []
[]

[Kernels]
  [mean_zero_pressure]
    type = ScalarLagrangeMultiplier
    variable = pressure
    lambda = lambda
  []

  [mass_matrix_vel_x]
    type = MassMatrix
    variable = vel_x
    matrix_tags = 'mass'
  []
  [mass_matrix_vel_y]
    type = MassMatrix
    variable = vel_y
    matrix_tags = 'mass'
  []
  [mass_matrix_pressure]
    type = MassMatrix
    variable = pressure
    matrix_tags = 'mass'
  []

  [u_jump]
    type = GradDiv
    matrix_only = true
    variable = vel_x
    u = vel_x
    v = vel_y
    component = 0
    vector_tags = ''
    matrix_tags = 'grad_div combined'
  []
  [v_jump]
    type = GradDiv
    matrix_only = true
    variable = vel_y
    u = vel_x
    v = vel_y
    component = 1
    vector_tags = ''
    matrix_tags = 'grad_div combined'
  []
[]

[ScalarKernels]
  [mean_zero_pressure_lm]
    type = AverageValueConstraint
    variable = lambda
    pp_name = pressure_integral
    value = 0
  []
[]

[DGKernels]
  [u_jump]
    type = MassFluxPenalty
    matrix_only = true
    variable = vel_x
    u = vel_x
    v = vel_y
    component = 0
    vector_tags = ''
    matrix_tags = 'jump combined'
  []
  [v_jump]
    type = MassFluxPenalty
    matrix_only = true
    variable = vel_y
    u = vel_x
    v = vel_y
    component = 1
    vector_tags = ''
    matrix_tags = 'jump combined'
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
    alpha = 6
    functor = '0'
    diffusivity = 'mu'
    component = 0
  []
  [momentum_x_diffusion_top]
    type = NavierStokesStressIPHDGDirichletBC
    boundary = 'top'
    variable = vel_x
    face_variable = vel_bar_x
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    alpha = 6
    functor = '${U}'
    diffusivity = 'mu'
    component = 0
  []
  [momentum_y_diffusion_all]
    type = NavierStokesStressIPHDGDirichletBC
    boundary = 'left bottom right top'
    variable = vel_y
    face_variable = vel_bar_y
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    alpha = 6
    functor = '0'
    diffusivity = 'mu'
    component = 1
  []

  [pressure_walls]
    type = MassContinuityIPHDGBC
    face_variable = pressure_bar
    variable = pressure
    boundary = 'left bottom right'
    face_velocity_functors = '0 0'
    interior_velocity_vars = 'vel_x vel_y'
  []
  [pressure_lid]
    type = MassContinuityIPHDGBC
    face_variable = pressure_bar
    variable = pressure
    boundary = 'top'
    face_velocity_functors = '${U} 0'
    interior_velocity_vars = 'vel_x vel_y'
  []

  [pb_mass]
    type = MassMatrixIntegratedBC
    variable = pressure_bar
    matrix_tags = 'mass'
    boundary = 'left right bottom top'
  []

  [u_jump_walls]
    type = MassFluxPenaltyBC
    matrix_only = true
    variable = vel_x
    face_variable = vel_bar_x
    u = vel_x
    v = vel_y
    component = 0
    vector_tags = ''
    matrix_tags = 'jump combined'
    boundary = 'left right bottom'
    face_velocity = 'walls'
    dirichlet_boundary = true
  []
  [v_jump_walls]
    type = MassFluxPenaltyBC
    matrix_only = true
    variable = vel_y
    face_variable = vel_bar_y
    u = vel_x
    v = vel_y
    component = 1
    vector_tags = ''
    matrix_tags = 'jump combined'
    boundary = 'left right bottom'
    face_velocity = 'walls'
    dirichlet_boundary = true
  []
  [u_jump_top]
    type = MassFluxPenaltyBC
    matrix_only = true
    variable = vel_x
    face_variable = vel_bar_x
    u = vel_x
    v = vel_y
    component = 0
    vector_tags = ''
    matrix_tags = 'jump combined'
    boundary = 'top'
    face_velocity = 'top'
    dirichlet_boundary = true
  []
  [v_jump_top]
    type = MassFluxPenaltyBC
    matrix_only = true
    variable = vel_y
    face_variable = vel_bar_y
    u = vel_x
    v = vel_y
    component = 1
    vector_tags = ''
    matrix_tags = 'jump combined'
    boundary = 'top'
    face_velocity = 'top'
    dirichlet_boundary = true
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

[FunctorMaterials]
  [top]
    type = ADGenericVectorFunctorMaterial
    prop_names = top
    prop_values = '${U} 0 0'
  []
  [walls]
    type = ADGenericVectorFunctorMaterial
    prop_names = walls
    prop_values = '0 0 0'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
  nl_rel_tol = 1e-12
[]

[Outputs]
  [out]
    type = CSV
    hide = 'pressure_integral lambda'
    execute_on = 'timestep_end'
  []
[]

[Postprocessors]
  [symmetric]
    type = MatrixSymmetryCheck
    execute_on = 'timestep_end'
  []
  [pressure_integral]
    type = ElementIntegralVariablePostprocessor
    variable = pressure
    execute_on = linear
  []
  [fe_jump_and_upb_equiv]
    type = MatrixEqualityCheck
    mat1 = 'vel_pb_grad_div.mat'
    mat2 = 'jump.mat'
  []
  [fe_grad_div_and_up_equiv]
    type = MatrixEqualityCheck
    mat1 = 'vel_p_grad_div.mat'
    mat2 = 'grad_div.mat'
  []
  [fe_combined_and_upall_equiv]
    type = MatrixEqualityCheck
    mat1 = 'vel_all_p_grad_div.mat'
    mat2 = 'combined.mat'
  []
  [upb_grad_div_num_zero_eig]
    type = MatrixEigenvalueCheck
    mat = vel_pb_grad_div.mat
  []
  [upb_div_grad_num_zero_eig]
    type = MatrixEigenvalueCheck
    mat = vel_pb_div_grad.mat
  []
  [up_grad_div_num_zero_eig]
    type = MatrixEigenvalueCheck
    mat = vel_p_grad_div.mat
  []
  [up_div_grad_num_zero_eig]
    type = MatrixEigenvalueCheck
    mat = vel_p_div_grad.mat
  []
  [upall_grad_div_num_zero_eig]
    type = MatrixEigenvalueCheck
    mat = vel_all_p_grad_div.mat
  []
  [upall_div_grad_num_zero_eig]
    type = MatrixEigenvalueCheck
    mat = vel_all_p_div_grad.mat
  []
[]
