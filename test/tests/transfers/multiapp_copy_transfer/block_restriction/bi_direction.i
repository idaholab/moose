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
    # Designed to have non-complete overlap
    subdomain_ids = '1 1 1 1
                     2 2 2 1
                     1 2 2 1
                     1 1 2 1'
  []
[]

[Variables]
  [to_sub]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 1
  []
  [from_sub]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 2
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

[MultiApps]
  [sub]
    type = TransientMultiApp
    input_files = sub.i
  []
[]

[Transfers]
  [to_sub]
    type = MultiAppCopyTransfer
    source_variable = to_sub
    variable = from_main
    to_multi_app = sub
  []
  [from_sub]
    type = MultiAppCopyTransfer
    source_variable = to_main
    variable = from_sub
    from_multi_app = sub
  []
[]

[Outputs]
  exodus = true
[]
