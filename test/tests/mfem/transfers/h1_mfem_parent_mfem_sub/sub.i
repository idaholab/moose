[Mesh]
  type = MFEMMesh
  file = ../../mesh/square.msh
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
  [u]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[BCs]
  [bottom]
    type = MFEMScalarDirichletBC
    variable = u
    boundary = 2
    coefficient = 1.0
  []
  [top]
    type = MFEMScalarDirichletBC
    variable = u
    boundary = 4
  []
[]

[Kernels]
  [diff]
    type = MFEMDiffusionKernel
    variable = u
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
  l_tol = 1e-16
  l_max_its = 1000
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[MultiApps]
  active = ''
  [subapp]
    type = FullSolveMultiApp
    input_files = parent.i
    execute_on = FINAL
  []
[]

[Transfers]
  active = ''
  [to_sub]
    type = MultiAppMFEMCopyTransfer
    source_variable = u
    variable = u
    to_multi_app = subapp
  []
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/DiffusionSub
    vtk_format = ASCII
  []
[]
