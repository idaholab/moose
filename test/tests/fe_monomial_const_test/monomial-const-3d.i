[Mesh]
  type = GeneratedMesh
  dim = 3
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  zmin = -1
  zmax = 1
  nx = 21
  ny = 21
  nz = 21
  elem_type = HEX8
[]

[Functions]
  [./bc_fn]
    type=ParsedFunction
    value=0
  [../]
  [./bc_fnt]
    type = ParsedFunction
    value = 0
  [../]
  [./bc_fnb]
    type = ParsedFunction
    value = 0
  [../]
  [./bc_fnl]
    type = ParsedFunction
    value = 0
  [../]
  [./bc_fnr]
    type = ParsedFunction
    value = 0
  [../]

  [./forcing_fn]
#    type = ParsedFunction
#    value = 0
    type = MTPiecewiseConst3D
  [../]

  [./solution]
    type = MTPiecewiseConst3D
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
    boundary = 'top bottom left right'
    function = bc_fn
  [../]
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
  nl_rel_tol = 1.e-9
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
