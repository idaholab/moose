# Diffusion on a line of six elements. Node 5 is constrained to a linear combination of
# nodes 1 and 3, which do not share an element, so the constraint has more than one primary
# node and its Jacobian entries lie outside the sparsity pattern of the mesh.

[Mesh]
  [line]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 6
  []
  allow_renumbering = false
[]

[Variables]
  [u]
    family = LAGRANGE
    order = FIRST
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
    value = 1
  []

  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 3
  []
[]

[Constraints]
  [c1]
    type = LinearNodalConstraint
    variable = u
    primary = '1 3'
    secondary_node_ids = 5
    penalty = 100
    weights = '0.25 0.75'
  []
[]

[Executioner]
  type = Steady

  solve_type = 'NEWTON'
[]
