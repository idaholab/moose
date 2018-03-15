[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 15
  ny = 15
  elem_type = QUAD4
[]

[GlobalParams]
  slope = 1
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
  active = 'dt'

  [./dt]
    type = TimestepSize
  [../]
[]

[Executioner]
  type = Transient


  solve_type = 'PJFNK'

  nl_rel_tol = 1e-7
#  l_tol = 1e-5

  [./Adaptivity]
    refine_fraction = 0.2
    coarsen_fraction = 0.3
    max_h_level = 4
  [../]

  start_time = 0.0
  end_time = 5
  num_steps = 500000

  dtmax = 0.25

  [./TimeStepper]
    type = DT2
    dt = 0.1
    e_max = 3e-1
    e_tol = 1e-1
  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  csv = false
  exodus = true
[]
