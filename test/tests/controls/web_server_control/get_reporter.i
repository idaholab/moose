# This should be ran by get_reporter.py to test
# getting a reporter value by the server

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[Controls]
  [web_server]
    type = WebServerControl
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 0.1234
[]

[Reporters]
  [constant]
    type = ConstantReporter
    real_names = 'pi'
    real_values = '${fparse pi}'
    real_vector_names = 'fibonacci'
    real_vector_values = '1 1 2 3 5 8 13'
    real_vector_vector_names = 'pascal'
    real_vector_vector_values = '1; 1 1; 1 2 1; 1 3 3 1'
    eigen_matrix_name = 'identity'
    eigen_matrix_value = '1 0;
                          0 1'
    integer_names = 'year'
    integer_values = '2025'
    string_names = 'name'
    string_values = 'Zach'
    point_names = '30_degrees'
    point_values = '${fparse cos(pi/6)} ${fparse sin(pi/6)} 1.0'
  []
[]

[Postprocessors]
  [tpp]
    type = TimePostprocessor
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[Outputs]
  [out]
    type = JSON
    postprocessors_as_reporters = true
  []
[]
