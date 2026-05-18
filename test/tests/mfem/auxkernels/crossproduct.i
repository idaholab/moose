[Mesh]
  type = MFEMMesh
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
  [HDivFESpace]
    type = MFEMVectorFESpace
    fec_type = RT
    fec_order = CONSTANT
  []
  [L2VectorFESpace]
    type = MFEMVectorFESpace
    fec_type = L2
    fec_order = CONSTANT
  []
[]

[Variables]
  [e_field]
    type = MFEMVariable
    fespace = HCurlFESpace
  []
  [b_field]
    type = MFEMVariable
    fespace = HDivFESpace
  []
[]

[AuxVariables]
  [lorentz_force]
    type = MFEMVariable
    fespace = L2VectorFESpace
  []
[]

[Functions]
  [external_e_field]
    type = ParsedVectorFunction
    expression_x = '1'
    expression_y = '0'
    expression_z = '0'
  []
  [external_b_field]
    type = ParsedVectorFunction
    expression_x = '0'
    expression_y = '1'
    expression_z = '0'
  []
[]

[ICs]
  [e_field_ic]
    type = MFEMVectorIC
    variable = e_field
    vector_coefficient = external_e_field
  []
  [b_field_ic]
    type = MFEMVectorIC
    variable = b_field
    vector_coefficient = external_b_field
  []
[]

[AuxKernels]
  [cross]
    type = MFEMCrossProductAux
    variable = lorentz_force
    first_source_vec = e_field
    second_source_vec = b_field
    execute_on = TIMESTEP_END
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[VectorPostprocessors]
  [line_sample_e_field]
    type = MFEMLineValueSampler
    variable = 'e_field'
    start_point = '0 0 0'
    end_point = '1 1 1'
    num_points = 101
    execute_on = 'final'
  []
  [line_sample_b_field]
    type = MFEMLineValueSampler
    variable = 'b_field'
    start_point = '0 0 0'
    end_point = '1 1 1'
    num_points = 101
    execute_on = 'final'
  []
  [line_sample_lorentz_force]
    type = MFEMLineValueSampler
    variable = 'lorentz_force'
    start_point = '0 0 0'
    end_point = '1 1 1'
    num_points = 101
    execute_on = 'final'
  []
[]

[Outputs]
  [CSV]
    type = CSV
    execute_on = 'final'
    file_base = OutputData/CrossProduct/crossproduct
  []
[]
