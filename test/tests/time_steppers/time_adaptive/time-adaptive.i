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
  [./u]
    order = FIRST
    family = LAGRANGE

    [./InitialCondition]
      type = TEIC
    [../]
  [../]
[]

[Kernels]
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
  [./all]
    type = TEJumpBC
    variable = u
    boundary = '0 1 2 3'
  [../]
[]

[Executioner]
  type = Transient

  [./TimeStepper]
    type = SolutionTimeAdaptiveDT
    dt = 0.5
  [../]

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  nl_abs_tol = 1e-15

  start_time = 0.0
  end_time = 5
  num_steps = 500000

  dtmin = 0.4
  dtmax = 0.9
[]

[Outputs]
  file_base = out
  csv = true
  exodus = true
[]
