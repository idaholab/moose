[Mesh]
  type = MFEMMesh
  file = ../mesh/mug.e
[]

[Problem]
  type = MFEMProblem
  solve = false
[]

[FESpaces]
  [H1VectorFESpace]
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
  [L2VectorFESpace]
    type = MFEMVectorFESpace
    fec_type = L2
    fec_order = CONSTANT
  []  
[]

[Variables]
  [h1_vector]
    type = MFEMVariable
    fespace = H1VectorFESpace
  []  
  [nd_vector]
    type = MFEMVariable
    fespace = HCurlFESpace
  []
  [rt_vector]
    type = MFEMVariable
    fespace = HDivFESpace
  []  
  [l2_vector]
    type = MFEMVariable
    fespace = L2VectorFESpace
  []  
[]

[Functions]
  [external_vector_field]
    type = ParsedVectorFunction
    expression_x = 'sin(kappa * y)'
    expression_y = 'sin(kappa * z)'
    expression_z = 'sin(kappa * x)'

    symbol_names = kappa
    symbol_values = 3.1415926535
  []  
[]

[ICs]
  [h1_vector_ic]
    type = MFEMVectorIC
    variable = h1_vector
    coefficient = external_vector_field
  []      
  [l2_vector_ic]
    type = MFEMVectorIC
    variable = l2_vector
    coefficient = external_vector_field
  []    
  [nd_vector_ic]
    type = MFEMVectorIC
    variable = nd_vector
    coefficient = external_vector_field
  []  
  [rt_vector_ic]
    type = MFEMVectorIC
    variable = rt_vector
    coefficient = external_vector_field
  []
[]

[Preconditioner]
  [boomeramg]
    type = MFEMHypreBoomerAMG
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/VectorIC
    vtk_format = ASCII
  []
[]
