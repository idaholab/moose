mu = 1
rho = 1
U = 1
n = 5
l = 1

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
  jump_matrices = 'jump grad_div combined'
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

  [pressure_convection]
    type = AdvectionIPHDGKernel
    variable = pressure
    face_variable = pressure_bar
    velocity = 'velocity'
    coeff = '${fparse -rho}'
    self_advection = false
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
  [pb_mass]
    type = MassMatrixDGKernel
    variable = pressure_bar
    matrix_tags = 'mass'
  []

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

  [mass_convection]
    type = AdvectionIPHDGPrescribedFluxBC
    face_variable = pressure_bar
    variable = pressure
    velocity = 'velocity'
    coeff = '${fparse -rho}'
    self_advection = false
    boundary = 'left bottom top right'
    prescribed_normal_flux = 0
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
    u = vel_x
    v = vel_y
    component = 0
    vector_tags = ''
    matrix_tags = 'jump combined'
    boundary = 'left right bottom'
    dirichlet_value = 'walls'
  []
  [v_jump_walls]
    type = MassFluxPenaltyBC
    matrix_only = true
    variable = vel_y
    u = vel_x
    v = vel_y
    component = 1
    vector_tags = ''
    matrix_tags = 'jump combined'
    boundary = 'left right bottom'
    dirichlet_value = 'walls'
  []
  [u_jump_top]
    type = MassFluxPenaltyBC
    matrix_only = true
    variable = vel_x
    u = vel_x
    v = vel_y
    component = 0
    vector_tags = ''
    matrix_tags = 'jump combined'
    boundary = 'top'
    dirichlet_value = 'top'
  []
  [v_jump_top]
    type = MassFluxPenaltyBC
    matrix_only = true
    variable = vel_y
    u = vel_x
    v = vel_y
    component = 1
    vector_tags = ''
    matrix_tags = 'jump combined'
    boundary = 'top'
    dirichlet_value = 'top'
  []
[]

[Functions]
  [top]
    type = ParsedVectorFunction
    value_x = ${U}
    value_y = 0
  []
  [walls]
    type = ParsedVectorFunction
    value_x = 0
    value_y = 0
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
