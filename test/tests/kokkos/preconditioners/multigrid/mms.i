[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  second_order = true
[]

[Variables]
  [u]
    family = LAGRANGE
    order = SECOND
  []
[]

[Kernels]
  [diff]
    type = KokkosDiffusion
    variable = u
  []
  [force]
    type = KokkosBodyForce
    variable = u
    value = 1
  []
[]

[BCs]
  [all]
    type = KokkosDirichletBC
    variable = u
    boundary = 'left right top bottom'
    value = 0
  []
[]

[Postprocessors]
  [L2u]
    type = ElementL2Norm
    variable = u
    execute_on = 'FINAL'
    outputs = 'csv'
  []
[]

[Preconditioning]
  [mg]
    type = MG
    orders = '1 2'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  use_kokkos_matrix_free_jacobian = true
[]

[Outputs]
  csv = true
[]
