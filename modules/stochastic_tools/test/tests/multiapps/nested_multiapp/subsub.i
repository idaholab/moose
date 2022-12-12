[Mesh/gmg]
  type = GeneratedMeshGenerator
  dim = 1
  nx = 10
  xmax = 1
[]

[Variables/u]
[]

[Kernels/diff]
  type = Diffusion
  variable = u
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
    value = 0
  []
[]

[Postprocessors]
  [lval]
    type = PointValue
    variable = u
    point = '0 0 0'
  []
  [rval]
    type = PointValue
    variable = u
    point = '1 0 0'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Controls/stm]
  type = SamplerReceiver
[]
