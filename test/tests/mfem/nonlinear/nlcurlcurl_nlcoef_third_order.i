# Manufactured solution for the nonlinear definite Maxwell problem
#   curl( k(|curl u|) curl u ) + u = f,   k(s) = 1 + s^2
# with exact solution u = (0, 0, sin(kappa*x)).  Then
#   curl u = (0, -kappa*cos(kappa*x), 0),  |curl u| = kappa*|cos(kappa*x)|
# and the forcing that reproduces it is
#   f_z = kappa^2*sin(kappa*x)*(1 + 3*kappa^2*cos(kappa*x)^2) + sin(kappa*x)
# where the trailing sin(kappa*x) is the contribution of the mass kernel.

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
    fec_order = THIRD
    closed_basis=GaussLobatto
    open_basis=IntegratedGLL
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
    expression_z = '(kappa^2) * sin(kappa * x) * (1 + 3 * (kappa^2) * (cos(kappa * x)^2) ) + sin(kappa * x)'

    symbol_names = kappa
    symbol_values = 3.1415926535
  []

  [k]
    type = MFEMParsedFunction
    expression = '1 + j^2'
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
    k_coefficient = k
    curlu_dk_dcurlu_coefficient = j_dk_dj
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

[Solvers]
  [matrix_free_ams]
    type = MFEMMatrixFreeAMS
    alpha_coefficient = k
    inner_pi_iterations = 4
    inner_g_iterations = 1
  []
  [lin]
    type = MFEMCGSolver
    preconditioner = matrix_free_ams
    l_tol = 1e-6
    l_max_its=5000
    print_level = 3
  []
  [native_mfem_nl]
    type = MFEMNewtonNonlinearSolver
    max_its = 100
    abs_tol = 1.0e-10
    rel_tol = 1.0e-10
    line_search = backtracking
  []
[]

[Executioner]
  type = MFEMSteady
  device = cuda
[]

[VectorPostprocessors]
  [line_sample_h_field]
    type = MFEMVariableLineValueSampler
    variable = 'h_field'
    start_point = '-0.99 -0.99 0.99'
    end_point = '0.99 0.99 -0.99'
    num_points = 114
  []
[]

[Postprocessors]
  [h_field_l2_error]
    type = MFEMVectorL2Error
    variable = h_field
    function = exact_h_field
  []
[]

[Outputs]
  [CSV]
    type = CSV
    file_base = OutputData/MFEMNonLinearCurlCurl
  []
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/MFEMNonLinearCurlCurl
    vtk_format = ASCII
    # optional:
    # refinements = 2            # oversample within each element so the ND field looks smoother
    # high_order_output = true   # write high-order cells (needs ParaView 5.5 or later)
    # scalar_coefficients = 'k'  # also write the nonlinear coefficient k(|curl h|)
  []
[]
