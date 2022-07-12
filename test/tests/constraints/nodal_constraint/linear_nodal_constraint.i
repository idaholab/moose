[Mesh]
  file = 2-lines.e
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
    boundary = 1
    value = 1
  []

  [right]
    type = DirichletBC
    variable = u
    boundary = 4
    value = 3
  []
[]

[Constraints]
  [c1]
    type = LinearNodalConstraint
    variable = u
    primary = 0
    secondary_node_ids = 4
    penalty = 100000
    weights = 10
  []
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]
