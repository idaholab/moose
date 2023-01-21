[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
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
    [./InitialCondition]
      type = ConstantIC
      value = 1
    [../]
  [../]
[]

[AuxVariables]
  [v]
    order = FIRST
    family = MONOMIAL
  []
[]

[Functions]
  [./forcing_fn]
    type = ParsedFunction
    expression = 2*pow(e,-x-(y*y))*(1-2*y*y)
  [../]
  [./exact_fn]
    type = ParsedGradFunction
    expression = pow(e,-x-(y*y))
    grad_x = -pow(e,-x-(y*y))
    grad_y = -2*y*pow(e,-x-(y*y))

  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./abs]          # u * v
    type = Reaction
    variable = u
  [../]
  [./forcing]
    type = BodyForce
    variable = u
    function = forcing_fn
  [../]
[]

[DGKernels]
  [regular_dg_diffusion]
    type = DGDiffusion
    variable = u
    epsilon = -1
    sigma = 6
  []
[]

[DGDiffusionAction]
  variable = u
  kernels_to_add = 'COUPLED'
  coupled_var = v
[]

[BCs]
  [./all]
    type = DGFunctionDiffusionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = exact_fn
    epsilon = -1
    sigma = 6
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
  [console]
    type = Console
    system_info = 'framework mesh aux nonlinear relationship execution'
  []
[]

[Problem]
  error_on_jacobian_nonzero_reallocation = true
[]

[Postprocessors]
  [num_rm]
    type = NumRelationshipManagers
  []
[]
