# Definite Maxwell problem solved with Nedelec elements of the first kind
# based on MFEM Example 3.

[Mesh]
  type = MFEMMesh
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

  ## note this is j * derivative, hence why it isnt just 2j
  [j_dk_dj]
    type = MFEMParsedFunction
    expression = '2*j^2'
    symbol_names = 'j'
    symbol_values = 'h_field_curl_mag'
  []
  # we need dk/ds / s in the finished kernel.
  # so we just input that here. it is usually
  # something nontrivial, but here it is just 2
  [dk_ds_s]
    type = MFEMParsedFunction
    expression = '2'
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
  [matrix_free_ams]
    type = MFEMMatrixFreeAMS
  []
  [jacobi]
    type = MFEMOperatorJacobiSmoother
  []
[]

[Solvers]
  [lin]
    type = MFEMGMRESSolver
    preconditioner = matrix_free_ams
    l_tol = 1e-16
    l_max_its = 10000
  []
  [native_mfem_nl]
    type = MFEMNewtonNonlinearSolver
    max_its = 100
    abs_tol = 1.0e-15
    rel_tol = 1.0e-15
    print_level = 1
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
  assembly_level = partial
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/NLCurlCurlMMSPartial
    vtk_format = ASCII
  []
[]
