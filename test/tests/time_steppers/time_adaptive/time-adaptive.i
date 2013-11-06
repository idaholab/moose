[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 20
  ny = 20
  elem_type = QUAD4
[]

[GlobalParams]
  slope = 10
  t_jump = 2
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE

    [./InitialCondition]
      type = TEIC
    [../]
  [../]
[]

[Kernels]
  active = 'td diff ffn'

  [./td]
    type = TimeDerivative
    variable = u
  [../]

  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./ffn]
    type = TEJumpFFN
    variable = u
  [../]
[]

[BCs]
  active = 'all'

  [./all]
    type = TEJumpBC
    variable = u
    boundary = '0 1 2 3'
  [../]
[]

[Postprocessors]
  active = ''

  [./dt]
    type = TimestepSize
  [../]
[]

[Executioner]
  type = Transient
  [./TimeStepper]
    type = SolutionTimeAdaptiveDT
    dt = 0.15
  [../]


  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  nl_abs_tol = 1e-15
#  l_tol = 1e-5

  start_time = 0.0
  end_time = 5
  num_steps = 500000

  dtmax = 0.1
  dtmax = 0.25
[]

[Output]
  file_base = out
  output_initial = true
  postprocessor_csv = true
  interval = 1
  exodus = true
  perf_log = true
[]
