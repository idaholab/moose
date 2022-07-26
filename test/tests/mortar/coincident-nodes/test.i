[Mesh]
  [top]
    type = GeneratedMeshGenerator
    dim = 2
    ymin = 0.5
    ymax = 1
    nx = 2
    ny = 1
    subdomain_ids = '0 0'
  []
  [bottom]
    type = GeneratedMeshGenerator
    dim = 2
    ymin = 0
    ymax = 0.5
    nx = 2
    ny = 1
    subdomain_ids = '1 1'
  []
  [combine]
    type = CombinerGenerator
    inputs = 'top bottom'
  []
  [secondary]
    type = LowerDBlockFromSidesetGenerator
    input = combine
    sidesets = left
    new_block_id = 11
    new_block_name = secondary
  []
  [primary]
    type = LowerDBlockFromSidesetGenerator
    input = secondary
    sidesets = right
    new_block_id = 12
    new_block_name = primary
  []
[]

[Problem]
  solve = false
[]

[Variables]
  [u][]
[]

[Constraints]
  [mortar]
    type = PenaltyEqualValueConstraint
    secondary_variable = u
    primary_boundary = right
    secondary_boundary = left
    primary_subdomain = 12
    secondary_subdomain = 11
    penalty_value = 10
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]
