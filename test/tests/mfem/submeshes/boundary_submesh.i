[Mesh]
  type = MFEMMesh
  file = ../mesh/cylinder-hex-q2.gen
  uniform_refine = 1
[]

[Problem]
  type = MFEMProblem
[]

[SubMeshes]
  [exterior]
    type = MFEMBoundarySubMesh
    boundary = 3
  []
[]

[FESpaces]
  [SubMeshH1FESpace]
    type = MFEMGenericFESpace
    fec_name = H1_2D_P1
    submesh = exterior
  []
[]

[Variables]
  [submesh_potential]
    type = MFEMVariable
    fespace = SubMeshH1FESpace
  []
[]

[Kernels]
  [mass]
    type = MFEMMassKernel
    variable = submesh_potential
  []
  [source]
    type = MFEMDomainLFKernel
    variable = submesh_potential
    coefficient = 3.0
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
  l_max_its = 1000
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/BoundaryPotential
    vtk_format = ASCII
    submesh = exterior
  []
[]
