# Definite Maxwell problem solved with Nedelec elements of the first kind
# based on MFEM Example 3.

[Mesh]
  type = MFEMMesh
  file = ../mesh/small_fichera.mesh
  dim = 3
[]

[Problem]
  type = MFEMProblem
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
[]

[Variables]
  [e_field]
    type = MFEMVariable
    fespace = HCurlFESpace
  []
[]

[AuxVariables]
  [db_dt_field]
    type = MFEMVariable
    fespace = HDivFESpace
  []
[]

[AuxKernels]
  [curl]
    type = MFEMCurlAux
    variable = db_dt_field
    source = e_field
    scale_factor = -1.0
    execute_on = TIMESTEP_END
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
    type = MFEMVectorFunctorTangentialDirichletBC
    variable = e_field
    vector_coefficient = exact_e_field
  []
[]

[Kernels]
  [curlcurl]
    type = MFEMCurlCurlKernel
    variable = e_field
    coefficient = 1.
  []
  [mass]
    type = MFEMVectorFEMassKernel
    variable = e_field
    coefficient = 1.
  []
  [source]
    type = MFEMVectorFEDomainLFKernel
    variable = e_field
    vector_coefficient = forcing_field
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
