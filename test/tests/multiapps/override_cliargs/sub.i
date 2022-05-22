[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
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
    value = 0
    boundary = left
  []

  [right]
    type = DirichletBC
    variable = u
    value = 1
    boundary = left
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [integral]
    type = ElementIntegralVariablePostprocessor
    variable = u
  []
[]

[Outputs]
  exodus = true
[]
