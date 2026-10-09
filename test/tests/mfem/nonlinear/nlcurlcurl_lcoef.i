# Definite Maxwell problem solved with Nedelec elements of the first kind
# based on MFEM Example 3.

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
    type = MFEMVectorTangentialDirichletBC
    variable = e_field
    vector_coefficient = exact_e_field
  []
[]

[Kernels]
  [curlcurl]
    type = MFEMNLCurlCurlKernel
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

[Solvers]
  [matrix_free_ams]
    type = MFEMMatrixFreeAMS
    # A single AMG V-cycle per auxiliary space keeps the preconditioner linear,
    # as plain GMRES requires. The default inner CG iterations do not.
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
    max_its = 10
    abs_tol = 1.0e-15
    rel_tol = 1.0e-15
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

 [VectorPostprocessors]
   [line_sample_e_field]
     type = MFEMVariableLineValueSampler
     variable = 'e_field'
     start_point = '-0.99 -0.99 0.99'
     end_point = '0.99 0.99 -0.99'
     num_points = 114
   []
 []

[Outputs]
  [CSV]
    type = CSV
    execute_on = 'timestep_end'
    file_base = OutputData/CurlCurl/curlcurl
  []
[]
