[Mesh]
  type = MFEMFileMesh
  file = ../../mesh/cube.e
[]

[Problem]
  type = MFEMProblem
  solve = false
  numeric_type = complex
[]

[FESpaces]
  [H1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
  []
  [HCurlFESpace]
    type = MFEMVectorFESpace
    fec_type = ND
    fec_order = FIRST
  []
[]

[Variables]
  [h1_variable]
    type = MFEMComplexVariable
    fespace = H1FESpace
  []
  [hcurl_variable]
    type = MFEMComplexVariable
    fespace = HCurlFESpace
  []
[]

[Functions]
  [h1_function_real]
    type = ParsedFunction
    expression = '5 * z'
  []
  [h1_function_imag]
    type = ParsedFunction
    expression = '10 * z'
  []
  [hcurl_function_real]
    type = ParsedVectorFunction
    expression_x = '2 * x'
    expression_y = '3 * y'
    expression_z = '4 * z'
  []
  [hcurl_function_imag]
    type = ParsedVectorFunction
    expression_x = '4 * x'
    expression_y = '6 * y'
    expression_z = '8 * z'
  []
[]

[ICs]
  [h1_variable_ic]
    type = MFEMComplexScalarIC
    variable = h1_variable
    coefficient_real = h1_function_real
    coefficient_imag = h1_function_imag
  []
  [hcurl_variable_ic]
    type = MFEMComplexVectorIC
    variable = hcurl_variable
    vector_coefficient_real = hcurl_function_real
    vector_coefficient_imag = hcurl_function_imag
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[VectorPostprocessors]
  [point_sampler]
    type = MFEMComplexVariablePointValueSampler
    variables = 'h1_variable hcurl_variable'
    points = '-0.375 -0.375 -0.375  0.375 0.375 0.375'
  []
[]

[Outputs]
  [CSV_point_value]
    type = CSV
    file_base = OutputData/complex_point_value_sampler_multi_variable
  []
[]
