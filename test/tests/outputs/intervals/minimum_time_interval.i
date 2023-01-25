[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 2
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
  []
  [time]
    type = TimeDerivative
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
  [dt]
    type = TimestepSize
  []
[]

[Executioner]
  type = Transient
  num_steps = 20
  dt = 0.1
  solve_type = NEWTON
[]

[Outputs]
  execute_on = 'timestep_end'
  [out]
    type = CSV
    minimum_time_interval = 0.21
  []
[]
