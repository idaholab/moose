# Definite Maxwell problem solved with Nedelec elements of the first kind
# based on MFEM Example 3.  

[Mesh]
  type = MFEMMesh
  file = gold/small_fichera.mesh
  dim = 3
[]

[Problem]
  type = MFEMProblem
[]

[FESpaces]
  [HCurlFESpace]
    type = MFEMFESpace
    fec_type = ND
    fec_order = FIRST
  []
[]

[Variables]
  [e_field]
    type = MFEMVariable
    fespace = HCurlFESpace
  []
[]

[Functions]
  [exact_e_field]
    type = ParsedVectorFunction
    expression_x = 'sin(kappa * y)'
    expression_y = 'sin(kappa * z)'
    expression_z = 'sin(kappa * x)'

    symbol_names = kappa
    symbol_values = 3.1415926535
  []

  [forcing_field]
    type = ParsedVectorFunction
    expression_x = '(1. + kappa * kappa) * sin(kappa * y)'
    expression_y = '(1. + kappa * kappa) * sin(kappa * z)'
    expression_z = '(1. + kappa * kappa) * sin(kappa * x)'

    symbol_names = kappa
    symbol_values = 3.1415926535
  []  
[]

[BCs]
  [tangential_E_bdr]
    type = MFEMVectorDirichletBC
    variable = e_field
    boundary = '1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24'
    vector_coefficient = TangentialECoefficient
  []
[]

[VectorCoefficients]
  [TangentialECoefficient]
    type = MFEMVectorFunctionCoefficient
    function = exact_e_field
  []
  [VolumetricSourceCoefficient]
    type = MFEMVectorFunctionCoefficient
    function = forcing_field
  []
[]


[Materials]
  [Substance]
    type = MFEMGenericConstantMaterial
    prop_names = one
    prop_values = 1.0
  []
[]

[Kernels]
  [curlcurl]
    type = MFEMCurlCurlKernel
    variable = e_field
    coefficient = one
  []
  [mass]
    type = MFEMVectorFEMassKernel
    variable = e_field
    coefficient = one
  []
  [source]
    type = MFEMVectorFEDomainLFKernel
    variable = e_field
    vector_coefficient = VolumetricSourceCoefficient
  []    
[]

[Preconditioner]
  [ams]
    type = MFEMHypreAMS
    fespace = HCurlFESpace
  []
[]

[Solver]
  type = MFEMHypreGMRES
  preconditioner = ams
  l_tol = 1e-6  
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/CurlCurl
    vtk_format = ASCII
  []
[]
