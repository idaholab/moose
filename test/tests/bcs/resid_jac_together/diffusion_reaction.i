[Mesh]
  [square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
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
  [nodal_bc]
    type = DirichletBC
    variable = u
    value = 1.2
    boundary = left
  []
  [integrated_bc]
    type = NeumannBC
    variable = u
    value = -2
    boundary = right
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  residual_and_jacobian_together = true
[]

[Outputs]
  exodus = true
[]
