# Block-restricted displacement on a multi-block mesh. The modifier moves only
# block 1 (the left half); nodes on the block 0/1 interface belong to block 1 and
# are displaced too. On a distributed mesh the interface can coincide with a
# partition boundary, exercising the node-position synchronization in snapNodes().
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
  []
  [left_block]
    type = SubdomainBoundingBoxGenerator
    input = gen
    block_id = 1
    bottom_left = '0 0 0'
    top_right = '0.5 1 0'
  []
[]

[Variables]
  [u]
    initial_condition = 1
  []
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Executioner]
  type = Steady
[]

[UserObjects]
  [move]
    type = MoveNodesByParsedExpression
    block = 1
    displacement_y = '0.1*sin(pi*x)'
    constant_names = 'pi'
    constant_expressions = '3.14159265358979'
    execute_on = 'INITIAL'
  []
[]

[Outputs]
  [out]
    type = Exodus
    execute_on = 'FINAL'
  []
[]
