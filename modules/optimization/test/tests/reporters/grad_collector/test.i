[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Reporters]
  [vals]
    type = ConstantReporter
    real_vector_names = 'parameters grad1a grad2a grad3a grad1b grad2b grad3b'
    real_vector_values = '1 2 3 4 5 6; 1 2; 3 4; 5 6; 1 2 3; 1 2 3 ;4 5 6'
    execute_on = INITIAL
  []
  [grad_collecta]
    type = ConstraintGradCollector
    parameter_reporter_name = vals/parameters
    gradient_reporter_names = 'vals/grad1a; vals/grad2a; vals/grad3a'
  []
  [grad_collectb]
    type = ConstraintGradCollector
    parameter_reporter_name = vals/parameters
    gradient_reporter_names = 'vals/grad1b; ;vals/grad3b'
  []
  [grad_collectc]
    type = ConstraintGradCollector
    parameter_reporter_name = vals/parameters
    gradient_reporter_names = ';vals/grad2b ;vals/grad3b'
  []
[]

[Outputs]
  [json]
    type = JSON
    execute_system_information_on = none
  []
[]

