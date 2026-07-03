
[Mesh]
  type = MFEMMesh
  file = ../mesh/cube_hex27.e
  dim = 3
  uniform_refine = 2
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
  [h_field]
    type = MFEMVariable
    fespace = HCurlFESpace
  []
[]

[Functions]
  [exact_h_field]
    type = ParsedVectorFunction
    expression_x = '0'
    expression_y = '0'
    expression_z = 'sin(kappa * x)'

    symbol_names = kappa
    symbol_values = 3.1415926535
  []

  [forcing_field]
    type = ParsedVectorFunction
    expression_x = '0'
    expression_y = '0'
    expression_z = 'sin(kappa * x) * (1 - 3 * (kappa^4) * (cos(kappa * x)^2) )'

    symbol_names = kappa
    symbol_values = 3.1415926535
  []

  [k]
    type = MFEMParsedFunction
    expression = 'j^2'
    symbol_names = 'j'
    symbol_values = 'h_field_curl_mag'
  []

  [j_dk_dj]
    type = MFEMParsedFunction
    expression = '2*j^2'
    symbol_names = 'j'
    symbol_values = 'h_field_curl_mag'
  []
[]

[BCs]
  [tangential_E_bdr]
    type = MFEMVectorTangentialDirichletBC
    variable = h_field
    vector_coefficient = exact_h_field
  []
[]

[Kernels]
  [curlcurl]
    type = MFEMNLCurlCurlKernel
    variable = h_field
  []
  [mass]
    type = MFEMVectorFEMassKernel
    variable = h_field
  []
  [source]
    type = MFEMVectorFEDomainLFKernel
    variable = h_field
    vector_coefficient = forcing_field
  []
[]

[Preconditioner]
  [ams]
    type = MFEMHypreAMS
    fespace = HCurlFESpace
  []
[]

[Solvers]
  [lin]
    type = MFEMHypreGMRES
    preconditioner = ams
    l_tol = 1e-12
  []
  [native_mfem_nl]
    type = MFEMNewtonNonlinearSolver
    max_its = 100
    abs_tol = 1.0e-10
    rel_tol = 1.0e-9
    print_level = 1
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/NLCurlCurl
    vtk_format = ASCII
  []
[]