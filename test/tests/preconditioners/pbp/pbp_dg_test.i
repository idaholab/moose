[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  elem_type = QUAD4
[]

[Variables]
  [./u]
    order = FIRST
    family = MONOMIAL
  [../]

  [./v]
    order = FIRST
    family = MONOMIAL
  [../]
[]

[Preconditioning]
  [./PBP]
    type = PBP
    solve_order = 'u v'
    preconditioner  = 'AMG AMG'
  [../]
[]

[Kernels]
  [./diff_u]
    type = Diffusion
    variable = u
  [../]

  [./abs_u]
    type = Reaction
    variable = u
  [../]

  [./forcing_u]
    type = BodyForce
    variable = u
    function = forcing_fn
  [../]

  [./diff_v]
    type = Diffusion
    variable = v
  [../]

  [./abs_v]
    type = Reaction
    variable = v
  [../]

  [./forcing_v]
    type = BodyForce
    variable = v
    function = forcing_fn
  [../]

  [./conv_v]
    type = CoupledForce
    variable = v
    v = u
  [../]
[]

[DGKernels]
  [./dg_diff]
    type = DGDiffusion
    variable = u
    epsilon = -1
    sigma = 6
  [../]

  [./dg_diff_2]
    type = DGDiffusion
    variable = v
    epsilon = -1
    sigma = 6
  [../]
[]

[Functions]
  [./forcing_fn]
    type = ParsedFunction
    expression = 2*pow(e,-x-(y*y))*(1-2*y*y)
  [../]

  [./exact_fn]
    type = ParsedGradFunction
    value = pow(e,-x-(y*y))
    grad_x = -pow(e,-x-(y*y))
    grad_y = -2*y*pow(e,-x-(y*y))
  [../]
[]

[BCs]
  [./all_u]
    type = DGFunctionDiffusionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = exact_fn
    epsilon = -1
    sigma = 6
  [../]

  [./all_v]
    type = DGFunctionDiffusionDirichletBC
    variable = v
    boundary = '0 1 2 3'
    function = exact_fn
    epsilon = -1
    sigma = 6
  [../]
[]

[Problem]
  type = FEProblem
  error_on_jacobian_nonzero_reallocation = true
[]

[Executioner]
  type = Steady

  l_max_its = 10
  nl_max_its = 10

  solve_type = JFNK
[]

[Outputs]
  exodus = true
[]
