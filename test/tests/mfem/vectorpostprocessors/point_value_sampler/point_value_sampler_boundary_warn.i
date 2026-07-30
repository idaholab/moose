# Sampling various variables that are not continuous across
# element boundaries in order to check that warnings are emitted.

[Mesh]
  type = MFEMMesh
  file = ../../mesh/mug.e
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
  [HDivFESpace]
    type = MFEMVectorFESpace
    fec_type = RT
    fec_order = CONSTANT
  []
  [L2FESpace]
    type = MFEMScalarFESpace
    fec_type = L2
    fec_order = CONSTANT
    basis = GaussLegendre
  []
[]

[Variables]
  [nd_vector]
    type = MFEMVariable
    fespace = HCurlFESpace
  []
  [rt_vector]
    type = MFEMVariable
    fespace = HDivFESpace
  []
  [l2_scalar]
    type = MFEMVariable
    fespace = L2FESpace
  []
[]

[Functions]
  [parsed_function]
    type = ParsedFunction
    expression = 'x + y*y + z*z*z'
  []
  [parsed_vector_function]
    type = ParsedVectorFunction
    expression_x = 'x'
    expression_y = 'y'
    expression_z = 'z'
  []
[]

[ICs]
  [nd_ic]
    type = MFEMVectorIC
    variable = nd_vector
    vector_coefficient = parsed_vector_function
  []
  [rt_ic]
    type = MFEMVectorIC
    variable = rt_vector
    vector_coefficient = parsed_vector_function
  []
  [l2_scalar_ic]
    type = MFEMScalarIC
    variable = l2_scalar
    coefficient = parsed_function
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[VectorPostprocessors]
  [point_sample]
    type = MFEMPointVariableValueSampler
    variable = 'l2_scalar'
    # this is a point very close to an internal element face
    points = '0.1 1e-13 -2.3125'
  []
[]
