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

[GPUKernels]
  [./diff]
    type = GPUCoefDiffusion
    variable = u
    coef = 0.1
  [../]
  [./time]
    type = GPUTimeDerivative
    variable = u
  [../]
[]

[GPUNodalKernels]
  [./td]
    type = GPUTimeDerivativeNodalKernel
    variable = nodal_ode
  [../]
  [./constant_rate]
    type = GPUConstantRate
    variable = nodal_ode
    rate = 1.0
  [../]
[]

[GPUBCs]
  [./left]
    type = GPUDirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = GPUDirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 20
  dt = 0.1
[]

[Outputs]
  exodus = true
[]
