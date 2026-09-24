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
    fec_order = FIRST
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
  # we need k'(s) / s in the finished kernel.
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
    k_coefficient = k
    curlu_dk_dcurlu_coefficient = j_dk_dj
    dk_dcurlu_over_curlu_coefficient = dk_ds_s
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
    inner_pi_iterations = 0
    inner_g_iterations = 0
  []
  [lin]
    type = MFEMGMRESSolver
    preconditioner = matrix_free_ams
    l_tol = 1e-16
  []
  [native_mfem_nl]
    type = MFEMNewtonNonlinearSolver
    max_its = 100
    abs_tol = 1.0e-15
    rel_tol = 1.0e-15
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
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
  # The L2 error agrees between assembly levels, so it is written on its own and both tests
  # diff the same gold. The sampled field goes to a separate CSV whose file base each test
  # suffixes, letting the two assembly levels carry different golds; they select different
  # integration rules, which moves the transverse components.
  [error]
    type = CSV
    file_base = OutputData/NLCurlCurlMMS
    execute_vector_postprocessors_on = NONE
  []
  [line_sample]
    type = CSV
    file_base = OutputData/NLCurlCurlMMS
    execute_postprocessors_on = NONE
  []
[]
