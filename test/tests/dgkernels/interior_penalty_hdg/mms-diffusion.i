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
  [ffn]
    type = BodyForce
    variable = u
    function = forcing
  []
[]

[DGKernels]
  [dg_interior]
    type = ADHDGDiffusion
    variable = u
    side_variable = side_u
    alpha = 6
  []
  [dg_side]
    type = ADHDGDiffusionSide
    variable = side_u
    interior_variable = u
    alpha = 6
  []
[]

[BCs]
  [dirichlet]
    type = HDGDiffusionBC
    variable = u
    exact_soln = exact
    boundary = 'left right top bottom'
    alpha = 6
  []
  [dirichlet_side]
    type = ADHDGSideDirichletBC
    variable = side_u
    exact_soln = exact
    boundary = 'left right top bottom'
  []
[]

[Functions]
  [exact]
    type = ParsedFunction
    expression = 'cos(.5*pi*x)*sin(.5*pi*y)'
  []
  [forcing]
    type = ParsedFunction
    expression = '.5*pi*.5*pi*cos(.5*pi*x)*sin(.5*pi*y) + .5*pi*.5*pi*cos(.5*pi*x)*sin(.5*pi*y)'
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
