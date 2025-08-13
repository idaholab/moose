[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax =  1
  ymin = -1
  ymax =  1
  nx = 2
  ny = 2
[]

[Problem]
    restart_file_base = steady_for_restart_out_cp/LATEST
[]

[Variables]
  [u]
  []
[]

[BCs]
  [all]
    type = DirichletBC
    variable = u
    boundary = 'left right top bottom'
    value = 0
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [ffn]
    type = BodyForce
    variable = u
    function = 2
  []
[]

[Executioner]
  type = Transient
  end_time = 2
  # By default, will start at
  # 1 since the end time from
  # the steady executioner is 1
  start_time = 0
  [./TimeStepper]
    type = TimeSequenceStepper
    time_sequence  = '0 1 2'
  [../]
[]
