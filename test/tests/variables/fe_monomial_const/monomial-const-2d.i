[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 100
  ny = 100
  elem_type = QUAD4
  # This test doesn't work in parallel with ParallelMesh because writing
  # CONSTANT MONOMIAL data doesn't currently work. See #2122.
  distribution = serial
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
    type = MTPiecewiseConst2D
  [../]

  [./solution]
    type = MTPiecewiseConst2D
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
  # Note: MOOSE's DirichletBCs do not work properly with shape functions that do not
  #       have DOFs at the element edges.  This test works because the solution
  #       has been designed to be zero at the boundary which is satisfied by the IC
  #       Ticket #1352
  active = ''
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


  nl_rel_tol = 1.e-10
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
