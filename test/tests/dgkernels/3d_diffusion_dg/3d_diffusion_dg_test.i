[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 5
  ny = 5
  nz = 5
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  zmin = 0
  zmax = 1
  elem_type = HEX8
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = MONOMIAL

    [./InitialCondition]
      type = ConstantIC
      value = 0.5
    [../]
  [../]
[]

[Functions]
  active = 'forcing_fn exact_fn'

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

[Kernels]
  active = 'diff abs forcing'

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
  active = 'dg_diff'

  [./dg_diff]
    type = DGDiffusion
    variable = u
    epsilon = -1
    sigma = 6
  [../]
[]

[BCs]
  active = 'all'

  [./all]
    type = DGFunctionDiffusionDirichletBC
    variable = u
    boundary = '0 1 2 3 4 5'
    function = exact_fn
    epsilon = -1
    sigma = 6
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Postprocessors]
  active = 'h dofs l2_err'

  [./h]
    type = AverageElementSize
    execute_on = 'initial timestep_end'
  [../]

  [./dofs]
    type = NumDOFs
    execute_on = 'initial timestep_end'
  [../]

  [./l2_err]
    type = ElementL2Error
    variable = u
    function = exact_fn
    execute_on = 'initial timestep_end'
  [../]
[]

[Outputs]
  file_base = out
  exodus = true
[]
