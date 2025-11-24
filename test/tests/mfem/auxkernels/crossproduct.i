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
    fec_map = INTEGRAL
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

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/CrossProduct
    vtk_format = ASCII
  []
[]
