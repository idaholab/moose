# Exercise 1 starting input: host MOOSE objects only
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 200
    ny = 200
  []
[]

[Variables]
  [T]
    initial_condition = 300
  []
[]

[Kernels]
  [time]
    type = TimeDerivative
    variable = T
  []
  [diff]
    type = Diffusion
    variable = T
  []
  [source]
    type = BodyForce
    variable = T
    value = 1e3
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = T
    boundary = left
    value = 300
  []
  [right]
    type = NeumannBC
    variable = T
    boundary = right
    value = -10
  []
[]

[Postprocessors]
  [T_avg]
    type = ElementAverageValue
    variable = T
  []
  [T_max]
    type = ElementExtremeValue
    variable = T
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = 0.1
  num_steps = 10
[]

[Outputs]
  csv = true
  perf_graph = true
[]
