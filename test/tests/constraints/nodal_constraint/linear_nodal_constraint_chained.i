# Diffusion on a line of six elements with two kinematic constraints in a chain: node 5 is
# constrained to node 3, and node 3 is constrained to node 1.

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
    primary = 3
    secondary_node_ids = 5
    penalty = 100
    weights = 1
    formulation = kinematic
  []
  [c2]
    type = LinearNodalConstraint
    variable = u
    primary = 1
    secondary_node_ids = 3
    penalty = 100
    weights = 1
    formulation = kinematic
  []
[]

[Executioner]
  type = Steady

  solve_type = 'NEWTON'
[]
