[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 10
  ny = 10
  elem_type = QUAD9
[]

[Functions]
  [./forcing_fn]
    type = ParsedFunction
    value = -2*(x*x+y*y-2)+(1-x*x)*(1-y*y)
  [../]

  [./solution]
    type = ParsedGradFunction
    value = (1-x*x)*(1-y*y)
    grad_x = 2*(x*y*y-x)
    grad_y = 2*(x*x*y-y)
[]

[Variables]
  [./u]
    order = SECOND
    family = HIERARCHIC
  [../]
[]

[Kernels]
  active = 'diff forcing reaction'
  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./reaction]
    type = Reaction
    variable = u
  [../]

  [./forcing]
    type = UserForcingFunction
    variable = u
    function = forcing_fn
  [../]
[]

[BCs]
  active = 'bc_all'
  [./bc_all]
    type = PenaltyDirichletBC
    variable = u
    value = 0
    function = solution
    boundary = 'top left right bottom'
  [../]
[]

[Postprocessors]
  [./dofs]
    type = PrintDOFs
  [../]

  [./h]
    type = AverageElementSize
    variable = u
  [../]

  [./L2error]
    type = ElementL2Error
    variable = u
    function = solution
  [../]
  [./H1error]
    type = ElementH1Error
    variable = u
    function = solution
  [../]
  [./H1Semierror]
    type = ElementH1SemiError
    variable = u
    function = solution
  [../]
[]

[Executioner]
  type = Steady
  petsc_options = '-snes_mf_operator'
  nl_rel_tol = 1e-15
[]

[Output]
  output_initial = false
  interval = 1
  exodus = true
  postprocessor_csv = true
  perf_log = true
[]
