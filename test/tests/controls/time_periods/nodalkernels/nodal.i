[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
  [../]
  [./nodal_ode]
  [../]
[]

[Kernels]
  [./diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
  [../]
  [./time]
    type = TimeDerivative
    variable = u
  [../]
[]

[NodalKernels]
  [./td]
    type = TimeDerivativeNodalKernel
    variable = nodal_ode
  [../]
  [./constant_rate]
    type = ConstantRate
    variable = nodal_ode
    rate = 1.0
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 10
  dt = 0.1
[]

[Controls]
  [./time_period]
    type = TimePeriod
    enable_objects = '*::constant_rate'
    start_time = 0.5
    end_time = 1
  [../]
[]

[Outputs]
  exodus = true
[]
