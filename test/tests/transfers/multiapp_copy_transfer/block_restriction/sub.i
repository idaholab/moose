[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 2
    ymin = 0
    ymax = 2
    nx = 4
    ny = 4
    subdomain_ids = '1 1 1 1
                     2 2 1 1
                     2 2 1 1
                     1 2 1 1'
  []
[]

[Variables]
  [to_main]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 4
  []
  [from_main]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 3
  []
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Outputs]
  exodus = true
[]
