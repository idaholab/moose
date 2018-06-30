[Mesh]
  type = GeneratedMesh
  nx = 10
  ny = 10
  dim = 2
[]

[Variables]
  [temp]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = temp
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = temp
    boundary = 'left'
    value = 0
  []
  [right]
    type = DirichletBC
    variable = temp
    boundary = 'right'
    value = 1
  []
[]

# No Executioner block
