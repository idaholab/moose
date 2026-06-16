[Mesh]
  [lefttest]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 20
    xmin = 0
    xmax = 1
    boundary_name_prefix = compact
    elem_type = EDGE2
  []
  [compact]
    type = SubdomainIDGenerator
    input = lefttest
    subdomain_id = 1
  []
  [righttest]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 20
    xmin = 2
    xmax = 3
    boundary_name_prefix = IR
    boundary_id_offset = 5
    elem_type = EDGE2
  []
  [IR]
    type = SubdomainIDGenerator
    input = righttest
    subdomain_id = 2
  []
  [full_test]
    type = MeshCollectionGenerator
    inputs = 'compact IR'
  []
  [block_rename]
    input = full_test
    type = RenameBlockGenerator
    old_block = '1 2'
    new_block = 'compact IR'
  []
  [left_interface]
    type = LowerDBlockFromSidesetGenerator
    input = block_rename
    sidesets = 'compact_right'
    new_block_id = 10
    new_block_name = 'interface_primary_subdomain'
  []
  [right_interface]
    type = LowerDBlockFromSidesetGenerator
    input = left_interface
    sidesets = 'IR_left'
    new_block_id = 20
    new_block_name = 'interface_secondary_subdomain'
  []
[]

[Variables]
  [u]
    initial_condition = 0
  []
  [lm]
    block = 'interface_secondary_subdomain'
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
    block = 'compact IR'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    boundary = compact_left
    variable = u
    value = 1
  []
  [right]
    type = DirichletBC
    boundary = IR_right
    variable = u
    value = 0
  []
[]

[Constraints]
  [mortar]
    type = EqualValueConstraint
    variable = lm
    secondary_variable = u
    primary_boundary = compact_right
    primary_subdomain = interface_primary_subdomain
    secondary_boundary = IR_left
    secondary_subdomain = interface_secondary_subdomain
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = false
[]
