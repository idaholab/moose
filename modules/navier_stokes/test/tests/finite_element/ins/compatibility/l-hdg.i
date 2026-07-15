[GlobalParams]
  u = vel_x
  v = vel_y
  grad_u = grad_vel_x
  grad_v = grad_vel_y
  face_u = face_vel_x
  face_v = face_vel_y
  pressure = pressure
  mu = 1
  rho = 1
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 10
    ymin = -1
    ymax = 1
    nx = 10
    ny = 2
    elem_type = TRI6
  []
[]

[Variables]
  [face_vel_x]
    family = SIDE_HIERARCHIC
    order = FIRST
  []
  [face_vel_y]
    family = SIDE_HIERARCHIC
    order = FIRST
  []
  [pressure]
    family = L2_LAGRANGE
    order = FIRST
  []
  [vel_x]
    family = L2_LAGRANGE
    order = FIRST
  []
  [vel_y]
    family = L2_LAGRANGE
    order = FIRST
  []
  [grad_vel_x]
    family = L2_LAGRANGE_VEC
    order = FIRST
  []
  [grad_vel_y]
    family = L2_LAGRANGE_VEC
    order = FIRST
  []
  [temperature]
    family = MONOMIAL
    order = FIRST
  []
  [grad_temperature]
    family = MONOMIAL_VEC
    order = FIRST
  []
  [face_temperature]
    family = SIDE_HIERARCHIC
    order = FIRST
  []
[]

[HDGKernels]
  [navier_stokes]
    type = NavierStokesLHDGKernel
  []
  [temperature_advection]
    type = AdvectionLHDGKernel
    variable = temperature
    face_variable = face_temperature
    velocity = velocity
    face_velocity = hybrid_velocity
    coeff = 1
  []
  [temperature_diffusion]
    type = DiffusionLHDGKernel
    u = temperature
    gradient_variable = grad_temperature
    face_variable = face_temperature
    diffusivity = thermal_conductivity
  []
[]

[Functions]
  [u_inlet]
    type = ParsedFunction
    expression = '1.5*(1-y*y)'
  []
[]

[BCs]
  [velocity_walls]
    type = NavierStokesLHDGVelocityDirichletBC
    boundary = 'bottom top'
    dirichlet_u = 0
    dirichlet_v = 0
  []
  [velocity_inlet]
    type = NavierStokesLHDGVelocityDirichletBC
    boundary = left
    dirichlet_u = u_inlet
    dirichlet_v = 0
  []
  [velocity_outlet]
    type = NavierStokesLHDGOutflowBC
    boundary = right
  []
  [temperature_advection_inlet]
    type = AdvectionLHDGDirichletBC
    boundary = left
    variable = temperature
    face_variable = face_temperature
    velocity = velocity
    face_velocity = inlet_velocity
    functor = 1
    coeff = 1
  []
  [temperature_diffusion_inlet]
    type = DiffusionLHDGDirichletBC
    boundary = left
    variable = temperature
    gradient_variable = grad_temperature
    face_variable = face_temperature
    diffusivity = thermal_conductivity
    functor = 1
  []
  [temperature_diffusion_insulated]
    type = DiffusionLHDGPrescribedGradientBC
    boundary = 'top bottom right'
    u = temperature
    gradient_variable = grad_temperature
    face_variable = face_temperature
    diffusivity = thermal_conductivity
    normal_gradient = 0
  []
  [temperature_advection_outlet]
    type = AdvectionLHDGOutflowBC
    boundary = right
    variable = temperature
    face_variable = face_temperature
    velocity = velocity
    face_velocity = hybrid_velocity
    coeff = 1
    constrain_lm = false
  []
[]

[FunctorMaterials]
  [hybrid_velocity]
    type = ADGenericVectorFunctorMaterial
    prop_names = hybrid_velocity
    prop_values = 'face_vel_x face_vel_y 0'
  []
  [inlet_velocity]
    type = ADGenericVectorFunctorMaterial
    prop_names = inlet_velocity
    prop_values = 'u_inlet 0 0'
  []
[]

[Materials]
  [constants]
    type = GenericConstantMaterial
    prop_names = 'mu thermal_conductivity'
    prop_values = '1 1'
  []
  [velocity]
    type = ADVectorFromComponentVariablesMaterial
    vector_prop_name = velocity
    u = vel_x
    v = vel_y
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       NONZERO               mumps'
  nl_rel_tol = 1e-12
[]

[Postprocessors]
  [outlet_velocity]
    type = SideAverageValue
    boundary = right
    variable = face_vel_x
  []
  [outlet_temperature]
    type = SideAverageValue
    boundary = right
    variable = face_temperature
  []
[]

[Outputs]
  csv = true
[]
