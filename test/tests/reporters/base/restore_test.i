[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [dt]
    type = TimeDerivative
    variable = u
  []
  [diff]
    type = Diffusion
    variable = u
  []
  [source]
    type = BodyForce
    variable = u
  []
[]

[Problem]
  verbose_restore = true
[]

[Reporters]
  [b]
    type = TestGetReporter
    int_reporter = a/int
    real_reporter = a/real
    vector_reporter = a/vector
    string_reporter = a/string
    broadcast_reporter = a/broadcast
    scatter_reporter = a/scatter
    gather_reporter = a/gather
  []
  [a]
    type = TestDeclareReporter
  []
[]

[Executioner]
  type = Transient
  end_time = 5
[]

[Problem]
  type = FailingProblem
  fail_steps = 5
[]
