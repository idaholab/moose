[Mesh]
  type = MFEMMesh
  file = ../mesh/cylinder-hex-q2.gen
[]

[Problem]
  type = MFEMProblem
[]

[SubMeshes]
  [wire]
    type = MFEMDomainSubMesh
    block = 1
  []
[]

[FESpaces]
  [H1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
  []
  [SubMeshH1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
    submesh = wire
  []
[]

[Variables]
  [electric_potential]
    type = MFEMVariable
    fespace = H1FESpace
  []
  [submesh_potential]
    type = MFEMVariable
    fespace = SubMeshH1FESpace
  []
[]

[BCs]
  [high_terminal]
    type = MFEMScalarDirichletBC
    variable = submesh_potential
    boundary = '1'
    coefficient = 1.0
  []
  [low_terminal]
    type = MFEMScalarDirichletBC
    variable = submesh_potential
    boundary = '2'
    coefficient = 0.0
  []
[]

[FunctorMaterials]
  [Substance]
    type = MFEMGenericFunctorMaterial
    prop_names = conductivity
    prop_values = 1.0
  []
[]

[Kernels]
  [diff]
    type = MFEMDiffusionKernel
    variable = submesh_potential
    coefficient = conductivity
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
  l_tol = 1e-20
  l_max_its = 1000
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Transfers]
  [submesh_potential_transfer]
    type = MFEMSubMeshTransfer
    from_variable = submesh_potential
    to_variable = electric_potential
  []
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/OpenCoilSourceSubMesh
    vtk_format = ASCII
    submesh = wire
  []
[]
