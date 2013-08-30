[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = -1
  xmax = 1
  nx = 100
  elem_type = EDGE3
  # This test doesn't work in parallel with ParallelMesh because writing
  # CONSTANT MONOMIAL data doesn't currently work. See #2122.
  distribution = serial
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
  # Note: MOOSE's DirichletBCs do not work properly with shape functions that do not
  #       have DOFs at the element edges.  This test works because the solution
  #       has been designed to be zero at the boundary which is satisfied by the IC
  #       Ticket #1352
  active = ''
  [./bc_all]
    type=FunctionDirichletBC
    variable = u
    boundary = 'left right'
    function = bc_fn
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
