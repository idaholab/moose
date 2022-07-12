[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
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

    # This exception won't be caught and will crash the simulation
    throw_std_exception = true
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
    preset = false
    boundary = 2
    value = 1
  [../]
  [./right2]
    type = DirichletBC
    variable = u
    preset = false
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
[]
