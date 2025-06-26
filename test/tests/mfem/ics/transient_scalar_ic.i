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
    coefficient = 1.0
  []
  [dh1_dt]
    type = MFEMTimeDerivativeMassKernel
    variable = h1_scalar
    coefficient = 1.0
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

[Preconditioner]
  [boomeramg]
    type = MFEMHypreBoomerAMG
  []
[]

[Solver]
  type = MFEMHypreGMRES
  preconditioner = boomeramg
  l_tol = 1e-8
  l_max_its = 100
[]

[Executioner]
  type = MFEMTransient
  device = cpu
  dt = 2.0
  start_time = 0.0
  end_time = 2.0
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/TransientScalarIC
    vtk_format = ASCII
  []
[]
