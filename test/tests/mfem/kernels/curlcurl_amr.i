# Definite Maxwell problem curl curl E + E = f on the unit square with exact solution
# E = (sin(pi y), sin(pi x)), with h-refinement driven by the L2 Zienkiewicz-Zhu indicator of the
# curl.

[Mesh]
  type = MFEMFileMesh
  file = ../mesh/inline-quad.mesh
  nonconforming = true
[]

[Problem]
  type = MFEMProblem
[]

[FESpaces]
  [HCurlFESpace]
    type = MFEMVectorFESpace
    fec_type = ND
    fec_order = SECOND
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
    expression_x = 'sin(pi * y)'
    expression_y = 'sin(pi * x)'
  []
  [forcing_field]
    type = ParsedVectorFunction
    expression_x = '(1 + pi^2) * sin(pi * y)'
    expression_y = '(1 + pi^2) * sin(pi * x)'
  []
[]

[BCs]
  [tangential_e]
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
  [source]
    type = MFEMVectorFEDomainLFKernel
    variable = e_field
    vector_coefficient = forcing_field
  []
[]

[Adaptivity]
  [Indicators]
    [l2zz]
      type = MFEML2ZienkiewiczZhuIndicator
      variable = e_field
      kernel = curlcurl
    []
  []
  [Markers]
    [ref]
      type = MFEMRefinementMarker
      threshold = 0.7
      indicator = l2zz
      max_h_level = 3
    []
  []
[]

[Solvers]
  [superlu]
    type = MFEMSuperLU
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
  [CSV]
    type = CSV
    file_base = OutputData/CurlCurlAMR
  []
[]
