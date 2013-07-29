[Mesh]
  type = GeneratedMesh
  dim = 3
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  zmin = -1
  zmax = 1
  nx = 5
  ny = 5
  zy = 5
  elem_type = HEX8
[]

[Functions]
  [./bc_fnt]
    type = ParsedFunction
    value = 1
  [../]
  [./bc_fnb]
    type = ParsedFunction
    value = -1
  [../]
  [./bc_fnl]
    type = ParsedFunction
    value = -1
  [../]
  [./bc_fnr]
    type = ParsedFunction
    value = 1
  [../]
  [./bc_fnf]
    type = ParsedFunction
    value = 1
  [../]
  [./bc_fnk]
    type = ParsedFunction
    value = -1
  [../]

  [./forcing_fn]
    type = ParsedFunction
    value = x+y+z
  [../]

  [./solution]
    type = ParsedGradFunction
    value = x+y+z
    grad_x = 1
    grad_y = 1
    grad_z = 1
[]

[Variables]
  [./u]
    order = FIRST
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
  petsc_options = '-snes_mf_operator'
[]

[Output]
  output_initial = false
  interval = 1
  exodus = true
  postprocessor_csv = true
  perf_log = true
[]
