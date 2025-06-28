[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 20
  []
[]

[Functions]
  [parameter_mesh]
    type = ParameterMeshFunction
    exodus_mesh = create_mesh_1d_out.e
    parameter_name = param_vec/params
    project_points = true
  []
[]

[VectorPostprocessors]
  [param_vec]
    type = CSVReaderVectorPostprocessor
    csv_file = create_mesh_1d_out_param_vec_0001.csv
  []
[]

[AuxVariables]
  [parameter]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [parameter_aux]
    type = FunctionAux
    function = parameter_mesh
    variable = parameter
  []
[]

[Outputs]
  exodus = true
  execute_on = timestep_end
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]
