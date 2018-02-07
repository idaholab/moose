[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
[]

[MultiApps]
  [./dummy]
    type = TransientMultiApp
    input_files = adaptiveDT.i
    execute_on = timestep_end
  [../]
[]

[Executioner]
  type = Transient
  start_time = 0
  end_time = 0.006
  dt = 0.006
  nl_abs_tol = 1.0e-8
[]

[Outputs]
  exodus = true
  file_base = end
[]
