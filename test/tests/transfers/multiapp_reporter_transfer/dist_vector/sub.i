[Mesh/generate]
  type = GeneratedMeshGenerator
  dim = 1
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Reporters]
  [from_main_rep]
    type = ConstantReporter
    real_vector_names = 'rec_vec'
    real_vector_values = '0.'
    real_names = 'rec_real'
    real_values = '-1.'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  [out]
    type = JSON
    execute_system_information_on = NONE
    execute_on = TIMESTEP_END
  []
[]
