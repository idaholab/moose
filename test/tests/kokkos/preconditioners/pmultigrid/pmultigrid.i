# The p-multigrid preconditioner's coarse levels are function spaces on the fine mesh, built from
# the fine system's variables at reduced order. A HIERARCHIC fine space makes the level a subset of
# the fine space: level p = 1 carries the vertex modes, which are the whole of the 25-dof bilinear
# space on this mesh, out of the 169 dofs of the cubic space.

[Mesh]
  [square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
    # HIERARCHIC shape functions above first order require a second-order geometric element
    elem_type = QUAD9
  []
[]

[Variables]
  [u]
    order = THIRD
    family = HIERARCHIC
  []
[]

[Kernels]
  [diff]
    type = KokkosDiffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = KokkosDirichletBC
    variable = u
    preset = false
    boundary = left
    value = 0
  []
  [right]
    type = KokkosDirichletBC
    variable = u
    preset = false
    boundary = right
    value = 1
  []
[]

[Preconditioning]
  [pmg]
    type = PMG
    level_orders = '1'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  use_kokkos_matrix_free_jacobian = true
[]

[Outputs]
  exodus = true
[]
