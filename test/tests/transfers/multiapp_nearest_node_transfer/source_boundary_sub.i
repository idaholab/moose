[Mesh]
  [drmg]
    type = DistributedRectilinearMeshGenerator
    dim = 2
    nx = 30
    ny = 30
    elem_type = QUAD4
    partition = square
  []
[]

[Variables]
  [u][]
[]

[Kernels]
  [conduction]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [right]
    type = DirichletBC
    variable = u
    boundary = 'right'
    value = 5
  []
  [left]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 0
  []
[]

[Executioner]
  type = Steady
  nl_rel_tol = 1e-6
[]

[Outputs]
  exodus = true
[]
