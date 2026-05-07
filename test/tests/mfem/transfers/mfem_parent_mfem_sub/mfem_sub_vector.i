[Mesh]
  type = MFEMMesh
  file = ../../mesh/cube.e
[]

[Problem]
  type = MFEMProblem
  solve = false
[]

[FESpaces]
  [H1FESpace]
    type = MFEMVectorFESpace
    fec_type = H1
    fec_order = FIRST
  []
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
    type = MFEMVectorFESpace
    fec_type = L2
    fec_order = CONSTANT
  []
[]

[Variables]
  [mfem_sub_h1_vector_var]
    type = MFEMVariable
    fespace = H1FESpace
  []
  [mfem_sub_hcurl_vector_var]
    type = MFEMVariable
    fespace = HCurlFESpace
  []
  [mfem_sub_hdiv_vector_var]
    type = MFEMVariable
    fespace = HDivFESpace
  []
  [mfem_sub_l2_vector_var]
    type = MFEMVariable
    fespace = L2FESpace
  []
[]

[Functions]
  [vector_field]
    type = ParsedVectorFunction
    expression_x = 'sin(pi * y)'
    expression_y = 'sin(pi * z)'
    expression_z = 'sin(pi * x)'
  []
[]

[ICs]
  [h1_vector_ic]
    type = MFEMVectorIC
    variable = mfem_sub_h1_vector_var
    vector_coefficient = vector_field
  []
  [hcurl_vector_ic]
    type = MFEMVectorIC
    variable = mfem_sub_hcurl_vector_var
    vector_coefficient = vector_field
  []
  [hdiv_vector_ic]
    type = MFEMVectorIC
    variable = mfem_sub_hdiv_vector_var
    vector_coefficient = vector_field
  []
  [l2_vector_ic]
    type = MFEMVectorIC
    variable = mfem_sub_l2_vector_var
    vector_coefficient = vector_field
  []
[]

[Executioner]
  type = Steady
[]
