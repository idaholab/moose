[Mesh]
  type = MFEMMesh
  file = ../mesh/torus.e
[]

[Problem]
  type = MFEMProblem
[]

[SubMeshes]
  [cut]
    type = MFEMBoundaryElementSubMesh
    boundary = 1
  []
[]

[FESpaces]
  [SubMeshH1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
    submesh = cut
  []
  [H1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
  []
[]

[Variables]
  [submesh_potential]
    type = MFEMVariable
    fespace = SubMeshH1FESpace
  []
[]

# High terminal set to new bdr attribute
[BCs]
  [high_terminal]
    type = MFEMScalarDirichletBC
    variable = submesh_potential
    boundary = '5' 
    value = 1.0
  []
  [low_terminal]
    type = MFEMScalarDirichletBC
    variable = submesh_potential
    boundary = '1'
    value = 0.0
  []
[]

[FunctorMaterials]
  [Substance]
    type = MFEMGenericFunctorMaterial
    prop_names = diffusivity
    prop_values = 1.0
  []
[]

[Kernels]
  [diff]
    type = MFEMDiffusionKernel
    variable = submesh_potential
    coefficient = diffusivity
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
    file_base = OutputData/CutPotential
    vtk_format = ASCII
    submesh = cut
  []
  [ParaViewDataCollection2]
    type = MFEMParaViewDataCollection
    file_base = OutputData/WholePotential
    vtk_format = ASCII
  []  
[]
