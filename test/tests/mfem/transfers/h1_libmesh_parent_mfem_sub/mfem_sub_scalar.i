[Mesh]
  type = MFEMMesh
  file = ../../mesh/square.e
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

[BCs]
  [sides]
    type = MFEMScalarDirichletBC
    variable = temperature
    coefficient = 1.0
  []
[]

[Kernels]
  [diff]
    type = MFEMDiffusionKernel
    variable = temperature
  []
  [source]
    type = MFEMDomainLFKernel
    variable = temperature
    coefficient = 2.0
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
  type = MFEMSteady
  device = cpu
[]
