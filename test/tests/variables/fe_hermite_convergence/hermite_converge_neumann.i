[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 4
  ny = 4
  elem_type = QUAD4
  # This test will not work in parallel with ParallelMesh enabled
  # due to a bug in PeriodicBCs.
  distribution = serial
[]

[Functions]
  [./bc_fn]
    type = ParsedGradFunction
    value = -sin(pi*x)*sin(pi*y)
    grad_x = -pi*cos(pi*x)*sin(pi*y)
    grad_y = -pi*sin(pi*x)*cos(pi*y)
  [../]

  [./bc_fnr]
    type = ParsedFunction
    value = -pi*cos(pi*x)*sin(pi*y)
  [../]
  [./bc_fnl]
    type = ParsedFunction
    value = pi*cos(pi*x)*sin(pi*y)
  [../]
  [./bc_fnt]
    type = ParsedFunction
    value = -pi*sin(pi*x)*cos(pi*y)
  [../]
  [./bc_fnb]
    type = ParsedFunction
    value = pi*sin(pi*x)*cos(pi*y)
  [../]

  [./forcing_fn]
    type = ParsedFunction
    value = -2*pi*pi*sin(pi*x)*sin(pi*y)-sin(pi*x)*sin(pi*y)
  [../]
[]

[Variables]
  [./u]
    order = THIRD
    family = HERMITE
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
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = 'bottom right top left'
    function = bc_fn
  [../]
  [./Periodic]
    [./all]
      variable = u
      auto_direction= 'x y'
    [../]
  [../]
  [./bc_right]
    type=FunctionNeumannBC
    variable = u
    boundary = 'right'
    function = bc_fnr
  [../]
  [./bc_left]
    type=FunctionNeumannBC
    variable = u
    boundary = 'left'
    function = bc_fnl
  [../]
  [./bc_top]
    type=FunctionNeumannBC
    variable = u
    boundary = 'top'
    function = bc_fnt
  [../]
  [./bc_bottom]
    type=FunctionNeumannBC
    variable = u
    boundary = 'bottom'
    function = bc_fnb
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
    function = bc_fn
  [../]
  [./H1error]
    type = ElementH1Error
    variable = u
    function = bc_fn
  [../]
  [./H1Semierror]
    type = ElementH1SemiError
    variable = u
    function = bc_fn
  [../]
[]

[Executioner]
  type = Steady
  petsc_options = '-snes_mf_operator'
  [./Adaptivity]
    steps = 2
    refine_fraction = 1
    coarsen_fraction = 0
    max_h_level = 10
    print_changed_info = 1
  [../]
  [./Quadrature]
    order=FIFTEENTH
  [../]
[]

[Output]
  output_initial = false
  interval = 1
  exodus = true
  postprocessor_csv = true
  perf_log = true
[]
