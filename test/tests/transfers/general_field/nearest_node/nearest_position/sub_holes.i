[Mesh]
  # Create a 4 rectangle pin lattice
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 100
    ny = 100
    xmax = 1
    ymax = 1
  []
  [pin_1]
    type = ParsedSubdomainMeshGenerator
    input = gmg
    combinatorial_geometry = 'x > 0.1 & x < 0.2 & y > 0.1 & y < 0.2'
    block_id = 1
  []
  [pin_2]
    type = ParsedSubdomainMeshGenerator
    input = pin_1
    combinatorial_geometry = 'x > 0.5 & x < 0.7 & y > 0.1 & y < 0.2'
    block_id = 2
  []
  [pin_3]
    type = ParsedSubdomainMeshGenerator
    input = pin_2
    combinatorial_geometry = 'x > 0.1 & x < 0.2 & y > 0.4 & y < 0.6'
    block_id = 3
  []
  [pin_4]
    type = ParsedSubdomainMeshGenerator
    input = pin_3
    combinatorial_geometry = 'x > 0.8 & x < 0.9 & y > 0.7 & y < 0.9'
    block_id = 4
  []
  [delete_back]
    type = BlockDeletionGenerator
    input = pin_4
    block = '0'
  []
[]

[AuxVariables]
  [to_main]
  []
  [to_main_elem]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[ICs]
  [pin_1]
    type = ConstantIC
    variable = to_main
    value = 1
    block = 1
  []
  [pin_2]
    type = ConstantIC
    variable = to_main
    value = 2
    block = 2
  []
  [pin_3]
    type = ConstantIC
    variable = to_main
    value = 3
    block = 3
  []
  [pin_4]
    type = ConstantIC
    variable = to_main
    value = 4
    block = 4
  []
  [pin_1_elem]
    type = ConstantIC
    variable = to_main_elem
    value = 1
    block = 1
  []
  [pin_2_elem]
    type = ConstantIC
    variable = to_main_elem
    value = 2
    block = 2
  []
  [pin_3_elem]
    type = ConstantIC
    variable = to_main_elem
    value = 3
    block = 3
  []
  [pin_4_elem]
    type = ConstantIC
    variable = to_main_elem
    value = 4
    block = 4
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Problem]
  solve = false
[]

[Outputs]
  [out]
    type = Exodus
    overwrite = true
  []
[]
