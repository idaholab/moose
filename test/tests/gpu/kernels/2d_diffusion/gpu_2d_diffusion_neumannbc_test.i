[Mesh]
  [square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  []
[]

[Variables]
  active = 'u'

  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[GPUKernels]
  active = 'diff'
  [diff]
    type = GPUDiffusion
    variable = u
  []
[]

[GPUBCs]
  active = 'left right'
  [left]
    type = GPUDirichletBC
    variable = u
    boundary = 3
    value = 0
  []
  [right]
    type = GPUNeumannBC
    variable = u
    boundary = 1
    value = 1
  []
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  file_base = neumannbc_out_gpu
  exodus = true
[]
