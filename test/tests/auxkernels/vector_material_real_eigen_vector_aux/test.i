[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 6
    ny = 6
  []
  [block]
    type = ParsedSubdomainMeshGenerator
    input = gmg
    combinatorial_geometry = 'x > 0.5'
    block_id = 1
  []
[]

[Materials]
  [foo_mat_block_0]
    type = GenericConstantArray
    prop_name = foo
    prop_value = '1 2'
    block = 0
  []
  [foo_mat_block_1]
    type = GenericConstantArray
    prop_name = foo
    prop_value = '3 4'
    block = 1
  []
[]

[AuxVariables/foo_mat_value]
  order = CONSTANT
  family = MONOMIAL
  components = 2
[]

[AuxKernels/foo_mat_value]
  type = VectorMaterialRealEigenVectorAux
  property = foo
  variable = foo_mat_value
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
