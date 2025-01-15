diff=2
a=2

[GlobalParams]
  variable = u
  face_variable = side_u
  diffusivity = ${diff}
  alpha = 6
  velocity = vel
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  elem_type = QUAD9
[]

[Variables]
  [u]
    order = FIRST
    family = MONOMIAL
  []
  [side_u]
    order = FIRST
    family = SIDE_HIERARCHIC
  []
[]

[Kernels]
  [ffn]
    type = BodyForce
    variable = u
    function = forcing
  []
[]

[HDGKernels]
  [diff]
    type = DiffusionIPHDGKernel
  []
  [adv]
    type = AdvectionIPHDGKernel
  []
[]

[BCs]
  [dirichlet_diff]
    type = DiffusionIPHDGDirichletBC
    functor = exact
    boundary = 'left right top bottom'
  []
  [dirichlet_adv]
    type = AdvectionIPHDGDirichletBC
    functor = exact
    boundary = 'left right top bottom'
  []
[]

[Materials]
  [vel]
    type = ADGenericConstantVectorMaterial
    prop_names = 'vel'
    prop_values = '${a} ${fparse 2*a} 0'
  []
[]

[Functions]
  [exact]
    type = ParsedFunction
    expression = 'sin(x)*cos(y)'
  []
  [forcing]
    type = ParsedFunction
    expression = '-2*a*sin(x)*sin(y) + a*cos(x)*cos(y) + 2*diff*sin(x)*cos(y)'
    symbol_names = 'a diff'
    symbol_values = '${a} ${diff}'
  []
[]

[Executioner]
  type = Steady
  nl_rel_tol = 1e-10
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  solve_type = NEWTON
[]

[Outputs]
  csv = true
[]

[Postprocessors]
  [h]
    type = AverageElementSize
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2u]
    type = ElementL2Error
    variable = u
    function = exact
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
[]
