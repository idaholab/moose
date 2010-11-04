[Mesh]
  dim = 2
  [./Generation]
    xmin = -1
    xmax = 1
    ymin = -1
    ymax = 1
    nx = 20
    ny = 20
    elem_type = QUAD4
  [../]
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

[Materials]
  active = empty

  [./empty]
    type = EmptyMaterial
    block = 0
  [../]
[]

[Postprocessors]
  active = 'dt'
  
  [./dt]
    type = PrintDT
  [../]
[]

[Executioner]
  type = DT2Transient
  
  perf_log = true
  petsc_options = '-snes_mf'

  nl_rel_tol = 1e-7
#  l_tol = 1e-5

  start_time = 0.0
  end_time = 5
  num_steps = 500000
  
  dt = 0.1
  dtmax = 0.25
  
  e_max = 3e-1
  e_tol = 1e-1
     
  perf_log = true
[]

[Output]
  file_base = out_dt2
  output_initial = false
  postprocessor_csv = true
  interval = 1
  exodus = true
[]
