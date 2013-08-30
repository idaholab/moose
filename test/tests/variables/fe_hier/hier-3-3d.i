[Mesh]
  type = GeneratedMesh
  dim = 3
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  zmin = -1
  zmax = 1
  nx = 1
  ny = 1
  nz = 1
  elem_type = HEX27
  # This problem only has 1 element, so using ParallelMesh in parallel
  # isn't really an option, and we don't care that much about ParallelMesh
  # in serial.
  distribution = serial
[]

[Functions]
  [./bc_fnt]
    type = ParsedFunction
    value = 3*y*y
  [../]
  [./bc_fnb]
    type = ParsedFunction
    value = -3*y*y
  [../]
  [./bc_fnl]
    type = ParsedFunction
    value = -3*x*x
  [../]
  [./bc_fnr]
    type = ParsedFunction
    value = 3*x*x
  [../]
  [./bc_fnk]
    type = ParsedFunction
    value = -3*z*z
  [../]
  [./bc_fnf]
    type = ParsedFunction
    value = 3*z*z
  [../]

  [./forcing_fn]
    type = ParsedFunction
    value = -6*x-6*y-6*z+(x*x*x)+(y*y*y)+(z*z*z)
  [../]

  [./solution]
    type = ParsedGradFunction
    value = (x*x*x)+(y*y*y)+(z*z*z)
    grad_x = 3*x*x
    grad_y = 3*y*y
    grad_z = 3*z*z
[]

[Variables]
  [./u]
    order = THIRD
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
  [./bc_front]
    type = FunctionNeumannBC
    variable = u
    boundary = 'front'
    function = bc_fnf
  [../]
  [./bc_back]
    type = FunctionNeumannBC
    variable = u
    boundary = 'back'
    function = bc_fnk
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
