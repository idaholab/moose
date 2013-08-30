[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 5
  ny = 5
  elem_type = QUAD9
[]

[Functions]
  [./bc_fnt]
    type = ParsedFunction
    value = 2*y
  [../]
  [./bc_fnb]
    type = ParsedFunction
    value = -2*y
  [../]
  [./bc_fnl]
    type = ParsedFunction
    value = -2*x
  [../]
  [./bc_fnr]
    type = ParsedFunction
    value = 2*x
  [../]

  [./forcing_fn]
    type = ParsedFunction
    value = -4+x*x+y*y
  [../]

  [./solution]
    type = ParsedGradFunction
    value = x*x+y*y
    grad_x = 2*x
    grad_y = 2*y
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
  [./bc_top]
    type = FunctionNeumannBC
    variable = u
    boundary = 'top'
    function = bc_fnt
  [../]
  [./bc_bottom]
    type = FunctionNeumannBC
    variable = u
    boundary = 'bottom'
    function = bc_fnb
  [../]
  [./bc_left]
    type = FunctionNeumannBC
    variable = u
    boundary = 'left'
    function = bc_fnl
  [../]
  [./bc_right]
    type = FunctionNeumannBC
    variable = u
    boundary = 'right'
    function = bc_fnr
  [../]
[]

[Postprocessors]
  [./dofs]
    type = NumDOFs
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

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'


[]

[Output]
  output_initial = false
  interval = 1
  exodus = true
  postprocessor_csv = true
  perf_log = true
[]
