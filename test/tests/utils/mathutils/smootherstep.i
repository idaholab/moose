[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  solve = false
[]

[Functions]
  [./smootherstep_value]
    type = SmootherStepTestFunction
  [../]
  [./exact_value]
    type = ParsedFunction
    expression = 'u := (x - 0.2) / (0.8 - 0.2);
             val := 6.0 * u^5 - 15 * u^4 + 10 * u^3;
             if(x < 0.2, 0, if(x > 0.8, 1, val))'
  [../]
  [./smootherstep_derivative]
    type = SmootherStepTestFunction
    derivative = true
  [../]
  [./exact_derivative]
    type = ParsedFunction
    expression = 'u := (x - 0.2) / (0.8 - 0.2);
             val := 30.0 * u^4 - 60 * u^3 + 30 * u^2;
             if(x < 0.2, 0, if(x > 0.8, 0, val / (0.8 - 0.2)))'
  [../]
[]

[VectorPostprocessors]
  [./functions]
    type = LineFunctionSampler
    functions = 'smootherstep_value exact_value smootherstep_derivative exact_derivative'
    start_point = '0 0 0'
    end_point = '1 0 0'
    num_points = 10
    sort_by = x
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
