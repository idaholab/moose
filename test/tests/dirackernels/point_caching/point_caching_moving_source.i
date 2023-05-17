[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 2
  ny = 2
  elem_type = QUAD4
  uniform_refine = 4
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  active = 'diff'

  [diff]
    type = Diffusion
    variable = u
  []
[]

[DiracKernels]
  # active = 'point_source'

  [point_source]
    type = ReporterPointSource
    variable = u
    allow_moving_sources = true
    value_name = base/source
    point_name = base/translation
  []
[]

[Reporters]
  [base]
    type = FunctorPointEvaluationReporter
    # the location of the point source
    point_vector_reporter_names = 'translation translation_2'
    point_vector_reporter_functors = '1 func_1 2 1 func_2 2; 1 func_1 2'
    point_vector_reporter_evaluation_points = '0 0 0 0 0 0'

    # the value of the two point sources
    real_vector_reporter_names = 'source'
    real_vector_reporter_functors = '1 func_2'
    real_vector_reporter_evaluation_points = '0 0 0'

    # random scalar to output to the console
    real_reporter_names = 'scalar'
    real_reporter_functors = '3'
    real_reporter_evaluation_points = '0 0 0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Functions]
  [func_1]
    type = ParsedFunction
    expression = '0.1 * t'
  []
  [func_2]
    type = ParsedFunction
    expression = 'x + 0.2 * t'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  []

  [right]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  []
[]

[Executioner]
  type = Transient
  num_steps = 2

  solve_type = 'PJFNK'
  nl_abs_tol = 1e-10
[]

[Outputs]
  exodus = true
  json = true
[]

[Debug]
  show_reporters = true
[]
