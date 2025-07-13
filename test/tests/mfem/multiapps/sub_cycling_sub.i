[Problem]
  type = MFEMProblem
[]

[Mesh]
  type = MFEMMesh
  file = ../mesh/square.e
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = MFEMDiffusionKernel
    variable = u
  [../]
  [./td]
    type = MFEMTimeDerivativeMassKernel
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = MFEMScalarDirichletBC
    variable = u
    boundary = left
    coefficient = 0
  [../]
  [./right]
    type = MFEMScalarDirichletBC
    variable = u
    boundary = right
    coefficient = 1
  [../]
[]

[Preconditioner]
  [boomeramg]
    type = MFEMHypreBoomerAMG
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
  num_steps = 2
  dt = 0.01
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/sub_cycling_sub
    vtk_format = ASCII
  []
[]
