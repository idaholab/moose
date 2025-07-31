[Problem]
  type = MFEMProblem
[]

[Mesh]
  type = MFEMMesh
  file = ../mesh/square.e
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = MFEMDiffusionKernel
    variable = u
  []
  [td]
    type = MFEMTimeDerivativeMassKernel
    variable = u
  []
[]

[BCs]
  [left]
    type = MFEMScalarDirichletBC
    variable = u
    boundary = left
    coefficient = 0
  []
  [right]
    type = MFEMScalarDirichletBC
    variable = u
    boundary = right
    coefficient = 1
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
  num_steps = 10
  dt = 1 # This will be constrained by the parent solve
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/dt_from_parent_sub
    vtk_format = ASCII
  []
[]
