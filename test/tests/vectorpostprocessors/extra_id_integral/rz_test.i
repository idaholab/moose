[Mesh]
  [cartesian_mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '0.5 0.5'
    dy = '1.0'
    ix = '2 2'
    iy = '1'
  []
  [cartesian_mesh_ids]
    type = ParsedExtraElementIDGenerator
    input = cartesian_mesh
    expression = 'if (x<0.5, 1, 2)'
    extra_elem_integer_name = 'element_id'
  []
  coord_type = 'RZ'
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[AuxVariables]
  [v]
    initial_condition = 1
  []
[]

[VectorPostprocessors]
  [average]
    type = ExtraIDIntegralVectorPostprocessor
    variable = 'v'
    id_name = 'element_id'
    average = true
  []
[]

[Outputs]
  csv = true
  execute_on = 'TIMESTEP_END'
[]
