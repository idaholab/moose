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
  [concentration]
    type = MFEMVariable
    fespace = H1FESpace
  []
  [complex]
    type = MFEMComplexVariable
    fespace = H1FESpace
  []
[]






[BCs]
  [bottom]
    type = MFEMScalarDirichletBC
    variable = concentration
    boundary = '1'
    coefficient = 1.0
  []
  [low_terminal]
    type = MFEMScalarDirichletBC
    variable = concentration
    boundary = '2'
    coefficient = 0.0
  []
[]

[Kernels]
  [diff]
    type = MFEMDiffusionKernel
    variable = concentration
    coefficient = 1.0
  []
[]

[Preconditioner]
  [jacobi]
    type = MFEMOperatorJacobiSmoother
  []
[]

[Solver]
  type = MFEMGMRESSolver
  preconditioner = jacobi
  l_tol = 1e-16
  l_max_its = 1000
[]

[Executioner]
  type = MFEMSteady
  device = cpu
  numeric_type = complex
[]
