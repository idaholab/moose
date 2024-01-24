[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  nz = 1
  xmin = 0.0
  xmax = 4.0
  ymin = 0.0
  ymax = 6.0
[]

[FunctorMaterials]
  [parsed_fmat]
    type = ParsedFunctorMaterial
    expression = 't + x + y + z'
    property_name = 'prop1'
    outputs = 'exodus'
    output_properties = 'prop1'
  []
  [parsed_vector_fmat]
    type = GenericVectorFunctorMaterial
    prop_names = 'prop1_vec'
    prop_values = '1 2 3'
    outputs = 'exodus'
    output_properties = 'prop1_vec'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
  # Get the t to be equal to 4
  time = 4.0
[]

[Outputs]
  exodus = true
  execute_on = 'INITIAL'
[]
