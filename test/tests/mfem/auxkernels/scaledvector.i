# Checks that MFEMScaledVectorAux projects the product of a scalar coefficient and a vector
# coefficient onto a vector auxvariable. Three scalar coefficients are covered: the default unit
# one, a spatially varying one given by a function, and one taking the value of a postprocessor.
# The vector coefficient is a variable in each case, so this also covers rescaling a vector
# variable. Each result is compared against the product written out explicitly as a vector
# function.

[Mesh]
  type = MFEMFileMesh
  file = ../mesh/ref-cube.mesh
[]

[Problem]
  type = MFEMProblem
  solve = false
[]

[FESpaces]
  [HCurlFESpace]
    type = MFEMVectorFESpace
    fec_type = ND
    fec_order = FIRST
  []
  # Linear discontinuous space, able to represent the product of the constant source field with
  # the linear scaling below exactly, so that the errors reported are at roundoff.
  [L2VectorFESpace]
    type = MFEMVectorFESpace
    fec_type = L2
    fec_order = FIRST
  []
[]

[Variables]
  [source_field]
    type = MFEMVariable
    fespace = HCurlFESpace
  []
[]

[AuxVariables]
  [unscaled]
    type = MFEMVariable
    fespace = L2VectorFESpace
  []
  [function_scaled]
    type = MFEMVariable
    fespace = L2VectorFESpace
  []
  [postprocessor_scaled]
    type = MFEMVariable
    fespace = L2VectorFESpace
  []
[]

[Functions]
  [source]
    type = ParsedVectorFunction
    expression_x = '1'
    expression_y = '2'
    expression_z = '3'
  []
  [ramp]
    type = ParsedFunction
    expression = '1 + x + 2*y + 3*z'
  []
  [expected_function_scaled]
    type = ParsedVectorFunction
    expression_x = '1 * (1 + x + 2*y + 3*z)'
    expression_y = '2 * (1 + x + 2*y + 3*z)'
    expression_z = '3 * (1 + x + 2*y + 3*z)'
  []
  [expected_postprocessor_scaled]
    type = ParsedVectorFunction
    expression_x = '2.5'
    expression_y = '5.0'
    expression_z = '7.5'
  []
[]

[ICs]
  [source_field_ic]
    type = MFEMVectorIC
    variable = source_field
    vector_coefficient = source
  []
[]

[AuxKernels]
  [copy]
    type = MFEMScaledVectorAux
    variable = unscaled
    vector_coefficient = source_field
    execute_on = TIMESTEP_END
  []
  [scale_by_function]
    type = MFEMScaledVectorAux
    variable = function_scaled
    vector_coefficient = source_field
    coefficient = ramp
    execute_on = TIMESTEP_END
  []
  [scale_by_postprocessor]
    type = MFEMScaledVectorAux
    variable = postprocessor_scaled
    vector_coefficient = source_field
    coefficient = Amplitude
    execute_on = TIMESTEP_END
  []
[]

[Postprocessors]
  # Coefficients built from postprocessor values are not ordered against the postprocessors
  # supplying them, so this is executed on an earlier flag than the aux kernel scaling by it.
  [Amplitude]
    type = ConstantPostprocessor
    value = 2.5
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [UnscaledError]
    type = MFEMVectorL2Error
    variable = unscaled
    function = source
  []
  [FunctionScaledError]
    type = MFEMVectorL2Error
    variable = function_scaled
    function = expected_function_scaled
  []
  [PostprocessorScaledError]
    type = MFEMVectorL2Error
    variable = postprocessor_scaled
    function = expected_postprocessor_scaled
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Outputs]
  csv = true
  file_base = OutputData/ScaledVector
[]
