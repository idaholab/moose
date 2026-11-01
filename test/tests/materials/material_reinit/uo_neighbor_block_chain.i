# An interface user object reads a neighbor property produced by a material that itself reads a
# property from another material on the neighbor block. Both neighbor-side producers must be computed.
# No solve is performed, so no other loop computes these materials.
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 1
    xmax = 2
  []
  [block1]
    type = SubdomainBoundingBoxGenerator
    input = gen
    bottom_left = '1 0 0'
    top_right = '2 1 0'
    block_id = 1
  []
  [interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = block1
    primary_block = 0
    paired_block = 1
    new_boundary = interface
  []
[]

[AuxVariables]
  [neighbor_value]
    order = CONSTANT
    family = MONOMIAL
    block = 0
  []
[]

[AuxKernels]
  [neighbor_value]
    type = InterfaceValueUserObjectAux
    variable = neighbor_value
    interface_uo_name = neighbor_value_uo
    boundary = interface
    execute_on = TIMESTEP_END
  []
[]

[UserObjects]
  [neighbor_value_uo]
    type = InterfaceQpMaterialPropertyRealUO
    property = D
    property_neighbor = D_chain
    boundary = interface
    interface_value_type = secondary
    execute_on = TIMESTEP_END
  []
[]

[Materials]
  [D_block0]
    type = GenericConstantMaterial
    block = 0
    prop_names = D
    prop_values = 4
  []
  # D_chain = 1 + A = 2 on block 1, only consumed through the neighbor side of the user object
  [A_block1]
    type = MaterialReinitTest
    block = 1
    property = A
    value = 1
  []
  [D_chain_block1]
    type = MaterialReinitTest
    block = 1
    property = D_chain
    value = 1
    coupled_property = A
  []
[]

[Postprocessors]
  # D_chain = 2 on the neighbor side of the interface (it would be 1 if A were skipped)
  [neighbor_value]
    type = ElementAverageValue
    variable = neighbor_value
    block = 0
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
