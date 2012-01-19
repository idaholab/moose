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
  slope = 1
  t_jump = 2
[]

[Functions]
  active = 'u_func'

  [./u_func]
    type = ParsedFunction
    value = 'atan((t-2)*pi)'   # atan((t-t_jump)*pi*slope) - has to match global params above

  [../]
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
  active = 'dt l2'

  [./dt]
    type = PrintDT
  [../]

  [./l2]
    type = ElementL2Error
    variable = u
    function = u_func
  [../]
[]

[Executioner]
  type = DT2Transient

  petsc_options = '-snes_mf_operator'

  nl_rel_tol = 1e-7
#  l_tol = 1e-5

  start_time = 0.0
  end_time = 5
  num_steps = 500000

  dt = 0.1
  dtmax = 0.25

  e_max = 3e-1
  e_tol = 1e-1
[]

[Output]
  file_base = out_dt2
  output_initial = false
  postprocessor_csv = true
  interval = 1
  exodus = true
  perf_log = true
[]
