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

[Executioner]
  type = Transient
  start_time = 0
  end_time = 0.006
  [./TimeStepper]
    type = IterationAdaptiveDT
    dt = 0.001
    optimal_iterations = 6
  [../]
  nl_abs_tol = 1.0e-8
[]

[Outputs]
  exodus = true
[]
