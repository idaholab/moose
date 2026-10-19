[Mesh]
  type = MFEMFileMesh
  file = ../../mesh/cube.e
[]

[Problem]
  type = MFEMProblem
  solve = false
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
    type = MFEMVariable
    fespace = H1FESpace
  []
  [hcurl_variable]
    type = MFEMVariable
    fespace = HCurlFESpace
  []
[]

[Functions]
  [h1_function]
    type = ParsedFunction
    expression = '5 * z'
  []
  [hcurl_function]
    type = ParsedVectorFunction
    expression_x = '2 * x'
    expression_y = '3 * y'
    expression_z = '4 * z'
  []
[]

[ICs]
  [h1_variable_ic]
    type = MFEMScalarIC
    variable = h1_variable
    coefficient = h1_function
  []
  [hcurl_variable_ic]
    type = MFEMVectorIC
    variable = hcurl_variable
    vector_coefficient = hcurl_function
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[VectorPostprocessors]
  [point_sampler]
    type = MFEMVariablePointValueSampler
    variables = 'h1_variable hcurl_variable'
    points = '-0.375 -0.375 -0.375  0.375 0.375 0.375'
  []
[]

[Outputs]
  [CSV_point_value]
    type = CSV
    file_base = OutputData/point_value_sampler_multi_variable
  []
[]
