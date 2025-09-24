mu = 1
rho = 1
l = 1
U = 100
n = 8

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
  [lambda]
    family = SCALAR
    order = FIRST
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
    alpha = 6
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
[]

[Kernels]
  [mean_zero_pressure]
    type = ScalarLagrangeMultiplier
    variable = pressure
    lambda = lambda
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
  [sc]
    type = StaticCondensation
    petsc_options_iname = '-pc_type -pc_factor_shift_type -ksp_view_pmat'
    petsc_options_value = 'lu       NONZERO               binary'
  []
[]

[Executioner]
  type = Steady
  nl_rel_tol = 1e-12
  petsc_options_iname = '-ksp_type'
  petsc_options_value = 'preonly'
[]

[Outputs]
  [out]
    type = Exodus
    hide = 'lambda pressure_integral symmetric vel_bar_x vel_bar_y pressure_bar'
  []
  [csv]
    type = CSV
    hide = 'lambda pressure_integral'
  []
[]

[Postprocessors]
  [Re]
    type = ParsedPostprocessor
    pp_names = ''
    expression = '${rho} * ${U} * ${l} / ${mu}'
  []
  [symmetric]
    type = MatrixSymmetryCheck
    mat = binaryoutput
    mat_number_to_load = 2
  []
  [pressure_integral]
    type = ElementIntegralVariablePostprocessor
    variable = pressure
    execute_on = linear
  []
[]
