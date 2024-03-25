diff=2
a=2

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
  [diff]
    type = Diffusion
    variable = u
  []
  [adv]
    type = ADConservativeAdvection
    variable = u
    velocity = vel
  []
  [ffn]
    type = BodyForce
    variable = u
    function = forcing
  []
[]

[DGKernels]
  [dg_interior_diff]
    type = ADHDGDiffusion
    variable = u
    side_variable = side_u
    alpha = 6
    diff = ${diff}
  []
  [dg_side_diff]
    type = ADHDGDiffusionSide
    variable = side_u
    interior_variable = u
    alpha = 6
    diff = ${diff}
  []
  [dg_interior_adv]
    type = ADHDGAdvection
    variable = u
    side_variable = side_u
    velocity = vel
  []
  [dg_side_adv]
    type = ADHDGAdvectionSide
    variable = side_u
    interior_variable = u
    velocity = vel
  []
[]

[BCs]
  [dirichlet_diff]
    type = HDGDiffusionBC
    variable = u
    exact_soln = exact
    boundary = 'left right top bottom'
    alpha = 6
    diff = ${diff}
  []
  [dg_interior_adv]
    type = ADHDGAdvectionDirichletBC
    variable = u
    exact_soln = exact
    velocity = vel
    boundary = 'left right top bottom'
  []
  [dirichlet_side]
    type = ADHDGSideDirichletBC
    variable = side_u
    exact_soln = exact
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
    expression = '-2*a*sin(x)*sin(y) + a*cos(x)*cos(y) + 2*diff*sin(x)*cos(y) + sin(x)*cos(y)'
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
