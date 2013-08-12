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
  [./bc_fnf]
    type = ParsedFunction
    value = 2*z
  [../]
  [./bc_fnk]
    type = ParsedFunction
    value = -2*z
  [../]

  [./forcing_fn]
    type = ParsedFunction
    value = -6+x*x+y*y+z*z
  [../]

  [./solution]
    type = ParsedGradFunction
    value = x*x+y*y+z*z
    grad_x = 2*x
    grad_y = 2*y
    grad_z = 2*z
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
  nl_rel_tol = 1e-11
  petsc_options = '-snes_mf_operator'
[]

[Output]
  output_initial = false
  interval = 1
  exodus = true
  postprocessor_csv = true
  perf_log = true
[]
