# Definite Maxwell problem curl(curl(u)) + u = f with f = curl(F) + u_e, where F = curl(u_e).
# The curl(F) part of the forcing is applied in its weak form (F, curl(v)), so the discrete
# solution converges to u_e.

[Mesh]
  type = MFEMFileMesh
  file = ../mesh/small_fichera.mesh
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
  [curl_exact_e_field]
    type = ParsedVectorFunction
    expression_x = '-kappa * cos(kappa * z)'
    expression_y = '-kappa * cos(kappa * x)'
    expression_z = '-kappa * cos(kappa * y)'
    symbol_names = kappa
    symbol_values = 3.1415926535
  []
[]

[BCs]
  [tangential_E_bdr]
    type = MFEMVectorTangentialDirichletBC
    variable = e_field
    vector_coefficient = exact_e_field
  []
[]

[Kernels]
  [curlcurl]
    type = MFEMCurlCurlKernel
    variable = e_field
  []
  [mass]
    type = MFEMVectorFEMassKernel
    variable = e_field
  []
  [curl_source]
    type = MFEMVectorFEDomainLFCurlKernel
    variable = e_field
    vector_coefficient = curl_exact_e_field
  []
  [mass_source]
    type = MFEMVectorFEDomainLFKernel
    variable = e_field
    vector_coefficient = exact_e_field
  []
[]

[Solvers]
  [ams]
    type = MFEMHypreAMS
    fespace = HCurlFESpace
  []
  [gmres]
    type = MFEMHypreGMRES
    preconditioner = ams
    l_tol = 1e-12
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Postprocessors]
  [l2_error]
    type = MFEMVectorL2Error
    variable = e_field
    function = exact_e_field
  []
[]

[Outputs]
  csv = true
  file_base = OutputData/CurlCurlLFCurl
[]
