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
  num_steps = 10
  dt = 0.2
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/dt_from_parent
    vtk_format = ASCII
  []
[]

[MultiApps]
  [sub_app]
    type = TransientMultiApp
    app_type = MooseTestApp
    input_files = 'dt_from_parent_sub.i'
  []
[]
