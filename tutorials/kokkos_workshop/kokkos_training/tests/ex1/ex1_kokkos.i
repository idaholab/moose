# Exercise 1 solution: every object replaced by its Kokkos counterpart
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
    type = KokkosTimeDerivative
    variable = T
  []
  [diff]
    type = KokkosDiffusion
    variable = T
  []
  [source]
    type = KokkosBodyForce
    variable = T
    value = 1e3
  []
[]

[BCs]
  [left]
    type = KokkosDirichletBC
    variable = T
    boundary = left
    value = 300
  []
  [right]
    type = KokkosNeumannBC
    variable = T
    boundary = right
    value = -10
  []
[]

[Postprocessors]
  [T_avg]
    type = KokkosElementAverageValue
    variable = T
  []
  [T_max]
    type = KokkosElementExtremeValue
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
