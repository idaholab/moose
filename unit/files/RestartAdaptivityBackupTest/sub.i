[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
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

[Adaptivity]
  initial_marker = uniform
  initial_steps = 1

  [Markers]
    [uniform]
      type = UniformMarker
      mark = refine
    []
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 0.1
[]
