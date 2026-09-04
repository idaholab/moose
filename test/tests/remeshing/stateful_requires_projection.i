[Mesh]
  [square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
    elem_type = TRI3
  []
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

[Materials]
  [stateful]
    type = StatefulMaterial
  []
[]

[Remeshing]
  [Criteria]
    [quality]
      type = ElementQualityCriterion
      quality_metric = MIN_ANGLE
      threshold = 0.3
    []
  []
  [Remeshers]
    [patch]
      type = PatchDelaunayRemesher
    []
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]
