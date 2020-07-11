[Mesh]
  file = 2squares.e
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./exception]
    type = ExceptionKernel
    variable = u
    when = residual
  [../]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./time_deriv]
    type = TimeDerivative
    variable = u
  [../]
[]

[BCs]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
  [./right2]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 5
  dt = 0.01
  dtmin = 0.005
  solve_type = 'PJFNK'
  snesmf_reuse_base = false
[]

[Outputs]
  exodus = true
[]
