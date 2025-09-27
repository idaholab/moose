mu = 1e-2
rho = 1
l = 1
U = 1
n = 1
degree = 1
alpha = '${fparse 10*degree^2}'

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
    family = L2_LAGRANGE
    order = FIRST
  []
  [vel_y]
    family = L2_LAGRANGE
    order = FIRST
  []
  # [pressure]
  #   family = MONOMIAL
  #   order = CONSTANT
  # []
  [vel_bar_x]
    family = LAGRANGE
    order = FIRST
  []
  [vel_bar_y]
    family = LAGRANGE
    order = FIRST
  []
  # [pressure_bar]
  #   family = LAGRANGE
  #   order = FIRST
  # []
  # [lambda]
  #   family = SCALAR
  #   order = FIRST
  # []
[]

[HDGKernels]
  # [momentum_x_convection]
  #   type = AdvectionIPHDGKernel
  #   variable = vel_x
  #   face_variable = vel_bar_x
  #   velocity = 'velocity'
  #   coeff = ${rho}
  # []
  [momentum_x_diffusion]
    # type = NavierStokesStressIPHDGKernel
    type = DiffusionIPHDGKernel
    variable = vel_x
    face_variable = vel_bar_x
    diffusivity = 'mu'
    alpha = ${alpha}
    # pressure_variable = pressure
    # pressure_face_variable = pressure_bar
    # component = 0
  []
  # [momentum_y_convection]
  #   type = AdvectionIPHDGKernel
  #   variable = vel_y
  #   face_variable = vel_bar_y
  #   velocity = 'velocity'
  #   coeff = ${rho}
  # []
  [momentum_y_diffusion]
    # type = NavierStokesStressIPHDGKernel
    type = DiffusionIPHDGKernel
    variable = vel_y
    face_variable = vel_bar_y
    diffusivity = 'mu'
    alpha = ${alpha}
    # pressure_variable = pressure
    # pressure_face_variable = pressure_bar
    # component = 1
  []
  # [pressure]
  #   type = MassContinuityIPHDGKernel
  #   variable = pressure
  #   face_variable = pressure_bar
  #   interior_velocity_vars = 'vel_x vel_y'
  #   face_velocity_functors = 'vel_bar_x vel_bar_y'
  # []
[]

# [Kernels]
#   [mean_zero_pressure]
#     type = ScalarLagrangeMultiplier
#     variable = pressure
#     lambda = lambda
#   []
# []

# [ScalarKernels]
#   [mean_zero_pressure_lm]
#     type = AverageValueConstraint
#     variable = lambda
#     pp_name = pressure_integral
#     value = 0
#   []
# []

[BCs]
  [momentum_x_diffusion_walls]
    # type = NavierStokesStressIPHDGDirichletBC
    type = DiffusionIPHDGDirichletBC
    boundary = 'left bottom right'
    variable = vel_x
    face_variable = vel_bar_x
    # pressure_variable = pressure
    # pressure_face_variable = pressure_bar
    alpha = ${alpha}
    functor = '0'
    diffusivity = 'mu'
    # component = 0
  []
  [momentum_x_diffusion_top]
    # type = NavierStokesStressIPHDGDirichletBC
    type = DiffusionIPHDGDirichletBC
    boundary = 'top'
    variable = vel_x
    face_variable = vel_bar_x
    # pressure_variable = pressure
    # pressure_face_variable = pressure_bar
    alpha = ${alpha}
    functor = '${U}'
    diffusivity = 'mu'
    # component = 0
  []
  [momentum_y_diffusion_all]
    # type = NavierStokesStressIPHDGDirichletBC
    type = DiffusionIPHDGDirichletBC
    boundary = 'left bottom right top'
    variable = vel_y
    face_variable = vel_bar_y
    # pressure_variable = pressure
    # pressure_face_variable = pressure_bar
    alpha = ${alpha}
    functor = '0'
    diffusivity = 'mu'
    # component = 1
  []
  # [momentum_x_advection_walls]
  #   type = AdvectionIPHDGDirichletBC
  #   variable = vel_x
  #   face_variable = vel_bar_x
  #   velocity = 'velocity'
  #   coeff = ${rho}
  #   boundary = 'left bottom right'
  #   functor = '0'
  # []
  # [momentum_x_advection_top]
  #   type = AdvectionIPHDGDirichletBC
  #   variable = vel_x
  #   face_variable = vel_bar_x
  #   velocity = 'velocity'
  #   coeff = ${rho}
  #   boundary = 'top'
  #   functor = '${U}'
  # []
  # [momentum_y_advection_all]
  #   type = AdvectionIPHDGDirichletBC
  #   variable = vel_y
  #   face_variable = vel_bar_y
  #   velocity = 'velocity'
  #   coeff = ${rho}
  #   boundary = 'left bottom right top'
  #   functor = '0'
  # []

  # [pressure_walls]
  #   type = MassContinuityIPHDGBC
  #   face_variable = pressure_bar
  #   variable = pressure
  #   boundary = 'left bottom right'
  #   face_velocity_functors = '0 0'
  #   interior_velocity_vars = 'vel_x vel_y'
  # []
  # [pressure_lid]
  #   type = MassContinuityIPHDGBC
  #   face_variable = pressure_bar
  #   variable = pressure
  #   boundary = 'top'
  #   face_velocity_functors = '${U} 0'
  #   interior_velocity_vars = 'vel_x vel_y'
  # []
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
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       NONZERO               mumps'
  nl_rel_tol = 1e-12
[]

[Outputs]
  [out]
    type = Exodus
    # hide = 'vel_bar_x vel_bar_y pressure_bar'
    hide = 'vel_bar_x vel_bar_y'
  []
[]

[Postprocessors]
  [Re]
    type = ParsedPostprocessor
    pp_names = ''
    expression = '${rho} * ${U} * ${l} / ${mu}'
  []
  # [pressure_integral]
  #   type = ElementIntegralVariablePostprocessor
  #   variable = pressure
  #   execute_on = linear
  # []
[]
