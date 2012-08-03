[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = -1
  xmax = 1
  nx = 100
  elem_type = EDGE3
[]

[Functions]
  [./bc_fn]
    type=ParsedFunction
    value=0
  [../]

  [./forcing_fn]
    type = MTPiecewiseConst1D
  [../]

  [./solution]
    type = MTPiecewiseConst1D
  [../]
[]

[Variables]
  [./u]
    order = CONSTANT
    family = MONOMIAL
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
    type=FunctionDirichletBC
    variable = u
    boundary = 'left right'
    function = bc_fn
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
  [./Adaptivity]

  [../]
[]

[Output]
  output_initial = false
  interval = 1
  exodus = true
  postprocessor_csv = true
  perf_log = true
[]
