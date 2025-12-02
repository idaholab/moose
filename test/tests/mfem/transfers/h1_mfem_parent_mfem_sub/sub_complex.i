[Mesh]
  type = MFEMMesh
  file = ../../mesh/square.msh
  dim = 2
[]

[Problem]
  type = MFEMProblem
  numeric_type = complex
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
    type = MFEMComplexVariable
    fespace = H1FESpace
  []
[]

[BCs]
  [bottom]
    type = MFEMComplexScalarDirichletBC
    variable = u
    boundary = 2
    coefficient_real = 1
    coefficient_imag = 2
  []
  [top]
    type = MFEMComplexScalarDirichletBC
    variable = u
    boundary = 4
  []
[]

[Kernels]
  [diff]
    type = MFEMComplexKernel
    variable = u
    [RealComponent]
      type = MFEMDiffusionKernel
    []
    [ImagComponent]
      type = MFEMDiffusionKernel
    []
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
    file_base = OutputData/DiffusionSubComplex
    vtk_format = ASCII
  []
[]
