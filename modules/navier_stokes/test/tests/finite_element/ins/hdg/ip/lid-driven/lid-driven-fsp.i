mu = 1
rho = 1
l = 1
U = 1e3
n = 200
gamma = 1e5

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
    diffusivity = 'mu'
    alpha = 6
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
  [mass_matrix_pressure]
    type = MassMatrix
    variable = pressure
    matrix_tags = 'mass'
    density = '${fparse -1/gamma}'
  []

  [grad_div_x]
    type = GradDiv
    variable = vel_x
    u = vel_x
    v = vel_y
    gamma = ${gamma}
    component = 0
  []
  [grad_div_y]
    type = GradDiv
    variable = vel_y
    u = vel_x
    v = vel_y
    gamma = ${gamma}
    component = 1
  []
[]

[DGKernels]
  [pb_mass]
    type = MassMatrixDGKernel
    variable = pressure_bar
    matrix_tags = 'mass'
    density = '${fparse -1/gamma}'
  []

  [u_jump]
    type = MassFluxPenalty
    variable = vel_x
    u = vel_x
    v = vel_y
    component = 0
    gamma = ${gamma}
  []
  [v_jump]
    type = MassFluxPenalty
    variable = vel_y
    u = vel_x
    v = vel_y
    component = 1
    gamma = ${gamma}
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
    density = '${fparse -1/gamma}'
  []

  [u_jump]
    type = MassFluxPenaltyBC
    variable = vel_x
    u = vel_x
    v = vel_y
    component = 0
    boundary = 'left right bottom top'
    gamma = ${gamma}
  []
  [v_jump]
    type = MassFluxPenaltyBC
    variable = vel_y
    u = vel_x
    v = vel_y
    component = 1
    boundary = 'left right bottom top'
    gamma = ${gamma}
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

[Preconditioning]
  [FSP]
    type = FSP
    topsplit = 'up'
    [up]
      splitting = 'u p'
      splitting_type = schur
      petsc_options_iname = '-pc_fieldsplit_schur_fact_type  -pc_fieldsplit_schur_precondition -ksp_gmres_restart -ksp_type -ksp_pc_side -ksp_rtol'
      petsc_options_value = 'full                            self                              300                fgmres    right        1e-4'
    []
    [u]
      vars = 'vel_x vel_y vel_bar_x vel_bar_y'
      petsc_options = '-ksp_converged_reason'
      petsc_options_iname = '-pc_type -ksp_type -ksp_rtol -ksp_gmres_restart -ksp_pc_side -pc_factor_mat_solver_type'
      petsc_options_value = 'ilu      gmres     1e-2      300                right        strumpack'
    []
    [p]
      vars = 'pressure pressure_bar'
      petsc_options = '-ksp_converged_reason'
      petsc_options_iname = '-ksp_type -ksp_gmres_restart -ksp_rtol -pc_type -ksp_pc_side'
      petsc_options_value = 'gmres     300                1e-2      ilu      right'
    []
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  active = ''
  [out]
    type = Exodus
    hide = 'pressure_average vel_bar_x vel_bar_y pressure_bar'
  []
[]

[Postprocessors]
  [Re]
    type = ParsedPostprocessor
    pp_names = ''
    expression = '${rho} * ${U} * ${l} / ${mu}'
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
