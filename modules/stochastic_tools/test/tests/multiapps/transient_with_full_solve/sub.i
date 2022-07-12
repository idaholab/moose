[Mesh/gen]
  type = GeneratedMeshGenerator
  dim = 1
  nx = 10
[]

[Variables/u]
[]

[Kernels/diff]
  type = ADDiffusion
  variable = u
[]

[BCs]
  [left]
    type = ADDirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = ADDirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors/center]
  type = PointValue
  point = '0.5 0 0'
  variable = u
[]

[Controls/stochastic]
  type = SamplerReceiver
[]
