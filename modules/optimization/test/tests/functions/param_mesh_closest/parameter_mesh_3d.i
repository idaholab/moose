[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 10
    ny = 10
    nz = 10
    zmax = 0
    zmin = -1
    xmin = -1
    xmax = 2
    ymax = 2
    ymin = -1
  []
[]

[Functions]
  [parameter_mesh]
    type = ParameterMeshFunction
    exodus_mesh = create_mesh_3d_out.e
    parameter_name = param_vec/params
    project_points = true
  []
[]

[VectorPostprocessors]
  [param_vec]
    type = CSVReaderVectorPostprocessor
    csv_file = create_mesh_3d_out_param_vec_0001.csv
  []
[]

[AuxVariables]
  [parameter]
    # family = MONOMIAL
    # order = CONSTANT
  []
[]

[AuxKernels]
  [parameter_aux]
    type = FunctionAux
    function = parameter_mesh
    variable = parameter
    execute_on = 'TIMESTEP_END'
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
