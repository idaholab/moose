[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  solve = false
[]

[Functions]
  [./constant]
    type = PolyTestFunction
    coefficients = '1'
  [../]
  [./constant_exact]
    type = ParsedFunction
    expression = '1'
  [../]
  [./quadratic]
    type = PolyTestFunction
    coefficients = '2 0 0'
  [../]
  [./quadratic_exact]
    type = ParsedFunction
    expression = '2 * x * x'
  [../]
  [./tenth]
    type = PolyTestFunction
    coefficients = '-1.0 0.9 -0.8 0.7 -0.6 0.5 -0.4 0.3 -0.2 0.1 -0.1'
  [../]
  [./tenth_exact]
    type = ParsedFunction
    expression = '-0.1 + 0.1 * x - 0.2 * x^2 + 0.3 * x^3 - 0.4 * x^4 + 0.5 * x^5 - 0.6 * x^6 + 0.7 * x^7 - 0.8 * x^8 + 0.9 * x^9 - 1.0 * x^10'
  [../]
  [./tenth_derivative]
    type = PolyTestFunction
    coefficients = '-1.0 0.9 -0.8 0.7 -0.6 0.5 -0.4 0.3 -0.2 0.1 -0.1'
    derivative = true
  [../]
  [./tenth_derivative_exact]
    type = ParsedFunction
    expression = '0.1 - 2.0 * 0.2 * x^1 + 3.0 * 0.3 * x^2 - 4.0 * 0.4 * x^3 + 5.0 * 0.5 * x^4 - 6.0 * 0.6 * x^5 + 7.0 * 0.7 * x^6 - 8.0 * 0.8 * x^7 + 9.0 * 0.9 * x^8 - 10.0 * 1.0 * x^9'
  [../]
[]

[VectorPostprocessors]
  [./out]
    type = LineFunctionSampler
    functions = 'constant constant_exact quadratic quadratic_exact tenth tenth_exact tenth_derivative tenth_derivative_exact'
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
