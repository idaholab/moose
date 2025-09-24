mu = 1
rho = 1

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 2
    ymin = -1
    ymax = 1
    nx = 2
    ny = 2
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
  [pressure_advection]
    type = AdvectionIPHDGKernel
    variable = pressure
    face_variable = pressure_bar
    velocity = 'velocity'
    coeff = '${fparse -rho}'
    self_advection = false
  []
[]

[Kernels]
  [u_ffn]
    type = BodyForce
    variable = vel_x
    function = forcing_u
  []
  [v_ffn]
    type = BodyForce
    variable = vel_y
    function = forcing_v
  []
  [p_ffn]
    type = BodyForce
    variable = pressure
    function = forcing_p
  []
[]

[BCs]
  # walls
  [momentum_x_diffusion_walls]
    type = NavierStokesStressIPHDGDirichletBC
    boundary = 'top bottom'
    variable = vel_x
    face_variable = vel_bar_x
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    alpha = 6
    functor = 'exact_u'
    diffusivity = 'mu'
    component = 0
  []
  [momentum_y_diffusion_walls]
    type = NavierStokesStressIPHDGDirichletBC
    boundary = 'top bottom'
    variable = vel_y
    face_variable = vel_bar_y
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    alpha = 6
    functor = 'exact_v'
    diffusivity = 'mu'
    component = 1
  []
  [mass_advection_walls]
    type = AdvectionIPHDGPrescribedFluxBC
    face_variable = pressure_bar
    variable = pressure
    velocity = 'velocity'
    coeff = '${fparse -rho}'
    self_advection = false
    boundary = 'top bottom'
    prescribed_normal_flux = 0
  []

  # inlet
  [momentum_x_advection_inlet]
    type = AdvectionIPHDGDirichletBC
    boundary = 'left'
    face_variable = vel_bar_x
    functor = exact_u
    variable = vel_x
    velocity = velocity
    coeff = ${rho}
  []
  [momentum_y_advection_inlet]
    type = AdvectionIPHDGDirichletBC
    boundary = 'left'
    face_variable = vel_bar_y
    functor = exact_v
    variable = vel_y
    velocity = velocity
    coeff = ${rho}
  []
  [momentum_x_diffusion_inlet]
    type = NavierStokesStressIPHDGDirichletBC
    boundary = 'left'
    variable = vel_x
    face_variable = vel_bar_x
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    alpha = 6
    functor = 'exact_u'
    diffusivity = 'mu'
    component = 0
  []
  [momentum_y_diffusion_inlet]
    type = NavierStokesStressIPHDGDirichletBC
    boundary = 'left'
    variable = vel_y
    face_variable = vel_bar_y
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    alpha = 6
    functor = 'exact_v'
    diffusivity = 'mu'
    component = 1
  []
  [mass_advection_inlet]
    type = AdvectionIPHDGPrescribedFluxBC
    face_variable = pressure_bar
    variable = pressure
    velocity = 'velocity'
    coeff = '${fparse -rho}'
    self_advection = false
    boundary = 'left'
    prescribed_normal_flux = 'negative_rhou'
  []

  # outlet
  [momentum_x_advection_outlet]
    type = AdvectionIPHDGOutflowBC
    boundary = 'right'
    variable = vel_x
    face_variable = vel_bar_x
    velocity = 'velocity'
    coeff = ${rho}
  []
  [momentum_y_advection_outlet]
    type = AdvectionIPHDGOutflowBC
    boundary = 'right'
    variable = vel_y
    face_variable = vel_bar_y
    velocity = 'velocity'
    coeff = ${rho}
  []
  [momentum_x_diffusion_outlet]
    type = NavierStokesStressIPHDGPrescribedFluxBC
    boundary = 'right'
    component = 0
    diffusivity = ${mu}
    face_variable = vel_bar_x
    prescribed_normal_flux = 0
    pressure_face_variable = pressure_bar
    pressure_variable = pressure
    variable = vel_x
  []
  [momentum_y_diffusion_outlet]
    type = NavierStokesStressIPHDGPrescribedFluxBC
    boundary = 'right'
    component = 1
    diffusivity = ${mu}
    face_variable = vel_bar_y
    prescribed_normal_flux = 0
    pressure_face_variable = pressure_bar
    pressure_variable = pressure
    variable = vel_y
  []
  [mass_outlet]
    type = AdvectionIPHDGOutflowBC
    boundary = 'right'
    variable = pressure
    face_variable = pressure_bar
    velocity = 'velocity'
    coeff = '${fparse -rho}'
    self_advection = false
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
  [exact_u]
    type = ParsedFunction
    expression = 'sin((1/2)*y*pi)*cos((1/2)*x*pi)'
  []
  [forcing_u]
    type = ParsedFunction
    expression = '(1/2)*pi^2*mu*sin((1/2)*y*pi)*cos((1/2)*x*pi) - 1/2*pi*rho*sin((1/4)*x*pi)*sin((1/2)*y*pi)^2*cos((1/2)*x*pi) + (1/2)*pi*rho*sin((1/4)*x*pi)*cos((1/2)*x*pi)*cos((1/2)*y*pi)^2 - pi*rho*sin((1/2)*x*pi)*sin((1/2)*y*pi)^2*cos((1/2)*x*pi) - 1/4*pi*sin((1/4)*x*pi)*sin((3/2)*y*pi)'
    symbol_names = 'mu rho'
    symbol_values = '${mu} ${rho}'
  []
  [exact_v]
    type = ParsedFunction
    expression = 'sin((1/4)*x*pi)*cos((1/2)*y*pi)'
  []
  [forcing_v]
    type = ParsedFunction
    expression = '(5/16)*pi^2*mu*sin((1/4)*x*pi)*cos((1/2)*y*pi) - pi*rho*sin((1/4)*x*pi)^2*sin((1/2)*y*pi)*cos((1/2)*y*pi) - 1/2*pi*rho*sin((1/4)*x*pi)*sin((1/2)*x*pi)*sin((1/2)*y*pi)*cos((1/2)*y*pi) + (1/4)*pi*rho*sin((1/2)*y*pi)*cos((1/4)*x*pi)*cos((1/2)*x*pi)*cos((1/2)*y*pi) + (3/2)*pi*cos((1/4)*x*pi)*cos((3/2)*y*pi)'
    symbol_names = 'mu rho'
    symbol_values = '${mu} ${rho}'
  []
  [exact_p]
    type = ParsedFunction
    expression = 'sin((3/2)*y*pi)*cos((1/4)*x*pi)'
  []
  [forcing_p]
    type = ParsedFunction
    expression = '-1/2*pi*rho*sin((1/4)*x*pi)*sin((1/2)*y*pi) - 1/2*pi*rho*sin((1/2)*x*pi)*sin((1/2)*y*pi)'
    symbol_names = 'rho'
    symbol_values = '${rho}'
  []
[]

[FunctorMaterials]
  [constant]
    type = GenericFunctorMaterial
    prop_names = 'rho'
    prop_values = '${rho}'
  []
  [rhou]
    type = ParsedFunctorMaterial
    expression = '-rho * exact_u'
    property_name = 'negative_rhou'
    functor_names = 'rho exact_u'
  []
[]

[Postprocessors]
  [l2u]
    type = ElementL2Error
    function = exact_u
    variable = vel_x
  []
  [l2v]
    type = ElementL2Error
    function = exact_v
    variable = vel_y
  []
  [l2p]
    type = ElementL2Error
    function = exact_p
    variable = pressure
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
    type = Exodus
    hide = 'pressure_bar vel_bar_x vel_bar_y l2u l2v l2p'
  []
[]
