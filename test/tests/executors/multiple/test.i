[Problem]
  solve = false
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
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

[Executors]
  [my_first_executor]
    type = BinaryTestExecutor
    inner1 = i1
    inner2 = i2
  []
  [i1]
    type = BinaryTestExecutor
  []
  [i2]
    type = BinaryTestExecutor
  []
[]

[Outputs]
  console = true
[]
