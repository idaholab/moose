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
    value = 2*pow(e,-x-(y*y))*(1-2*y*y)
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

  [./abs]					# u * v
    type = Reaction
    variable = u
  [../]

  [./forcing]
    type = UserForcingFunction
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
    boundary = '0 1 2 3'
    function = exact_fn
    epsilon = -1
    sigma = 6
  [../]
[]

[Executioner]
  type = Steady
  perf_log = true

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
[]

[Postprocessors]
  active = 'h dofs l2_err'

  [./h]
    type = AverageElementSize
    variable = u
  [../]

  [./dofs]
    type = NumDOFs
    variable = u
  [../]

  [./l2_err]
    type = ElementL2Error
    variable = u
    function = exact_fn
  [../]
[]

[Output]
  file_base = out
  output_initial = true
  interval = 1
  exodus = true
[]
