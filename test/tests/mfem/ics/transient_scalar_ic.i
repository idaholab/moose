[Mesh]
  type = MFEMMesh
  file = ../mesh/cylinder-hex-q2.gen
[]

[Problem]
  type = MFEMProblem
[]

[FESpaces]
  [H1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
  []
  [L2FESpace]
    type = MFEMScalarFESpace
    fec_type = L2
    fec_order = CONSTANT
    basis = GaussLegendre
  []
[]

[Variables]
  [h1_scalar]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[AuxVariables]
  [l2_scalar]
    type = MFEMVariable
    fespace = L2FESpace
  []
[]

[Functions]
  [height]
    type = ParsedFunction
    expression = 'z'
  []
[]

[ICs]
  [l2_scalar_ic]
    type = MFEMScalarIC
    variable = l2_scalar
    coefficient = 2.0
  []
  [h1_scalar_ic]
    type = MFEMScalarIC
    variable = h1_scalar
    coefficient = height
  []
[]

[Kernels]
  [h1_laplacian]
    type = MFEMDiffusionKernel
    variable = h1_scalar
  []
  [dh1_dt]
    type = MFEMTimeDerivativeMassKernel
    variable = h1_scalar
  []
[]

[BCs]
  [bottom]
    type = MFEMScalarDirichletBC
    variable = h1_scalar
    boundary = '1'
    coefficient = height
  []
  [top_dirichlet]
    type = MFEMScalarDirichletBC
    variable = h1_scalar
    boundary = '2'
    coefficient = height
  []
[]


[Solvers]
  [boomeramg]
    type = MFEMHypreBoomerAMG
  []
  [main]
    type = MFEMHypreGMRES
    preconditioner = boomeramg
    use_initial_guess = true # problem is solved by initial condition
  []
[]

[Executioner]
  type = MFEMTransient
  device = cpu
  dt = 2.0
  start_time = 0.0
  end_time = 2.0
[]

[VectorPostprocessors]
  [line_sample_h1_scalar]
    type = MFEMLineValueSampler
    variable = 'h1_scalar'
    start_point = '-1 0 -0.5'
    end_point = '1 0 0.5'
    num_points = 101
  []
  [line_sample_l2_scalar]
    type = MFEMLineValueSampler
    variable = 'l2_scalar'
    start_point = '-0.99 -0.01 -0.49'
    end_point = '0.99 0.01 0.49'
    num_points = 114
  []
[]

[Outputs]
  [CSV]
    type = CSV
    execute_on = 'timestep_end'
    file_base = OutputData/TransientScalarIC/transient_scalar_ic
  []
[]
