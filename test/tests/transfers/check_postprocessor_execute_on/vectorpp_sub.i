[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 1
  ymax = 2
[]

[Problem]
  kernel_coverage_check = false
[]

[Variables]
  [./u]
  [../]
[]

[Postprocessors]
  [./receive]
    type = Receiver
  [../]

  [./send]
    type = ScalePostprocessor
    value = receive
    scaling_factor = 2
  [../]
[]

[Executioner]
  type = Transient
  nl_abs_tol = 1e-10
  num_steps = 1
[]
