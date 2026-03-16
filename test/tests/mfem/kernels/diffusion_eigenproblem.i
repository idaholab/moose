[Mesh]
  type = MFEMMesh
  file = ../mesh/star.mesh
  serial_refine = 3
  dim = 3
[]

[Problem]
  type = MFEMEigenproblem
  num_modes = 5
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
  [all]
    type = MFEMScalarDirichletBC
    variable = u
    coefficient = 1.0
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
    print_level = 0
  []
[]

[Solver]
  type = MFEMHypreLOBPCG
  preconditioner = boomeramg
  print_level = 0
  coefficient = 1.0
  l_tol = 1e-10
  l_max_its = 300
  random_seed = 11
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/DiffEigenproblem
    vtk_format = ASCII
  []
[]
