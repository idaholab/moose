# A nodal RankTwoAux, which recovers its value from the quadrature points of the elements around each
# node, reads a tensor produced by a material that reads a scalar from a material that reads another
# scalar. All three producers must be computed on every element of the patch.
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 3
[]

[AuxVariables]
  [t00]
  []
[]

[AuxKernels]
  [t00]
    type = RankTwoAux
    variable = t00
    rank_two_tensor = tensor
    index_i = 0
    index_j = 0
  []
[]

[Materials]
  # tensor(0,0) = s = 1 + A = 3, only consumed through the auxiliary kernel
  [A]
    type = GenericConstantMaterial
    prop_names = A
    prop_values = 2
  []
  [s]
    type = ParsedMaterial
    property_name = s
    material_property_names = A
    expression = '1 + A'
  []
  [tensor]
    type = RankTwoTensorFromComponentProperties
    tensor_name = tensor
    tensor_values = 's 0 0 0 0 0 0 0 0'
  []
[]

[Postprocessors]
  [t00]
    type = NodalExtremeValue
    variable = t00
    value_type = min
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
  # enough quadrature points for a first order patch at corner nodes, which see a single element
  [Quadrature]
    order = THIRD
  []
[]

[Outputs]
  csv = true
[]
