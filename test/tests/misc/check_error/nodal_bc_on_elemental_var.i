[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
 []

[Variables]
  [u]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [bcs]
    type = DirichletBC
    variable = u
    boundary = 'left right'
    value = 1
  []
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 1
  abort_on_solve_fail = true
[]
