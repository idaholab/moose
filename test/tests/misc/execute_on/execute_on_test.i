[Mesh]
  type = GeneratedMesh
  dim = 2
  xmax = 1
  ymax = 1
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  []

  [right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  []
[]

[Reporters]
  [initial]
    type = CurrentExecFlagReporter
    execute_on = initial
  []
  [timestep_begin]
    type = CurrentExecFlagReporter
    execute_on = timestep_begin
  []
  [timestep_end]
    type = CurrentExecFlagReporter
    execute_on = timestep_end
  []
  [nonlinear]
    type = CurrentExecFlagReporter
    execute_on = nonlinear
  []
  [linear]
    type = CurrentExecFlagReporter
    execute_on = linear
  []
  [custom]
    type = CurrentExecFlagReporter
    execute_on = custom
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  [out]
    type = JSON
  []
[]
