[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = ADDiffusion
    variable = u
  []
  [time]
    type = ADTimeDerivative
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Postprocessors]
  [average]
    type = AverageNodalVariableValue
    variable = u
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 0.25
  solve_type = NEWTON
[]

[Controls]
  [stochastic]
    type = SamplerReceiver
  []
[]
