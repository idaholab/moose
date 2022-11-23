[Mesh]
  file = 2-lines.e
  allow_renumbering = false
[]

[Variables]
  [u]
    family = LAGRANGE
    order = FIRST
    block = 1
  []
  [v]
    family = LAGRANGE
    order = FIRST
    block = 2
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
    block = 1
  []
  [diff2]
    type = Diffusion
    variable = v
    block = 2
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  []

  [right_v]
    type = DirichletBC
    variable = v
    boundary = 4
    value = 3
  []
[]

[Constraints]
  [c1]
    type = LinearNodalConstraint
    variable = u
    variable_secondary = v
    primary = 0
    secondary_node_ids = 4
    penalty = 100000
    weights = 10
  []
[]

[Executioner]
  type = Steady

  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]
