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
  [mfem_parent_h1_vector_var]
    type = MFEMVariable
    fespace = H1FESpace
  []
  [mfem_parent_hcurl_vector_var]
    type = MFEMVariable
    fespace = HCurlFESpace
  []
  [mfem_parent_hdiv_vector_var]
    type = MFEMVariable
    fespace = HDivFESpace
  []
  [mfem_parent_l2_vector_var]
    type = MFEMVariable
    fespace = L2FESpace
  []      
[]

[AuxVariables]
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
    variable = mfem_parent_h1_vector_var
    vector_coefficient = vector_field
  []
  [hcurl_vector_ic]
    type = MFEMVectorIC
    variable = mfem_parent_hcurl_vector_var
    vector_coefficient = vector_field
  []
  [hdiv_vector_ic]
    type = MFEMVectorIC
    variable = mfem_parent_hdiv_vector_var
    vector_coefficient = vector_field
  []
  [l2_vector_ic]
    type = MFEMVectorIC
    variable = mfem_parent_l2_vector_var
    vector_coefficient = vector_field
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[MultiApps]
  [mfem_app]
    type = FullSolveMultiApp
    input_files = mfem_sub_vector.i
    execute_on = 'INITIAL'
  []
[]

[Transfers]
  [h1_transfer_from_subapp]
    type = MultiAppMFEMGeneralFieldTransfer
    source_variable = mfem_sub_h1_vector_var
    variable = mfem_sub_h1_vector_var
    from_multi_app = mfem_app
  []
  [hcurl_transfer_from_subapp]
    type = MultiAppMFEMGeneralFieldTransfer
    source_variable = mfem_sub_hcurl_vector_var
    variable = mfem_sub_hcurl_vector_var
    from_multi_app = mfem_app
  []
  [hdiv_transfer_from_subapp]
    type = MultiAppMFEMGeneralFieldTransfer
    source_variable = mfem_sub_hdiv_vector_var
    variable = mfem_sub_hdiv_vector_var
    from_multi_app = mfem_app
  []
  [l2_transfer_from_subapp]
    type = MultiAppMFEMGeneralFieldTransfer
    source_variable = mfem_sub_l2_vector_var
    variable = mfem_sub_l2_vector_var
    from_multi_app = mfem_app
  []  
[]

[Postprocessors]
  [H1_Var_L2_Error]
    type = MFEMVectorL2Error
    variable = mfem_parent_h1_vector_var
    function = mfem_sub_h1_vector_var
    execute_on = TIMESTEP_END
  []
  [HCurl_Var_L2_Error]
    type = MFEMVectorL2Error
    variable = mfem_parent_hcurl_vector_var
    function = mfem_sub_hcurl_vector_var
    execute_on = TIMESTEP_END
  []  
  [HDiv_Var_L2_Error]
    type = MFEMVectorL2Error
    variable = mfem_parent_hdiv_vector_var
    function = mfem_sub_hdiv_vector_var
    execute_on = TIMESTEP_END
  []
  [L2_Var_L2_Error]
    type = MFEMVectorL2Error
    variable = mfem_parent_l2_vector_var
    function = mfem_sub_l2_vector_var
    execute_on = TIMESTEP_END
  []
[]

[Outputs]
  file_base = 'mfem_parent_mfem_sub_vector_h1_hcurl_hdiv_l2_hex'
  csv = true  
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/TEST
    vtk_format = ASCII
  []
[]