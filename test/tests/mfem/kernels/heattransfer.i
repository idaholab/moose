[Mesh]
  type = MFEMMesh
  file = ../mesh/mug.e
  dim = 3
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
[]

[Variables]
  [temperature]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[Kernels]
  [diff]
    type = MFEMDiffusionKernel
    variable = temperature
  []
  [dT_dt]
    type = MFEMTimeDerivativeMassKernel
    variable = temperature
  []
[]

[BCs]
  active = 'bottom top_convective'
  [bottom]
    type = MFEMScalarDirichletBC
    variable = temperature
    boundary = '1'
    coefficient = 1.0
  []
  [top_convective]
    type = MFEMConvectiveHeatFluxBC
    variable = temperature
    boundary = '2'
    T_infinity = .5
    heat_transfer_coefficient = 5
  []
  [top_dirichlet]
    type = MFEMScalarDirichletBC
    variable = temperature
    boundary = '2'
  []
[]

[Preconditioner]
  [boomeramg]
    type = MFEMHypreBoomerAMG
  []
  [jacobi]
    type = MFEMOperatorJacobiSmoother
  []
[]

[Solver]
  type = MFEMHypreGMRES
  preconditioner = boomeramg
  l_tol = 1e-16
  l_max_its = 1000
[]

[Executioner]
  type = MFEMTransient
  device = cpu
  assembly_level = legacy
  dt = 2.0
  start_time = 0.0
  end_time = 6.0
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/HeatTransfer
    vtk_format = ASCII
  []
[]
