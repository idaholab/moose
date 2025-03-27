[Mesh]
  type = MFEMMesh
  file = gold/mug.e
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
  [diffused]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[BCs]
  [bottom]
    type = MFEMScalarDirichletBC
    variable = diffused
    boundary = '1'
    value = 1.0
  []
  [low_terminal]
    type = MFEMScalarDirichletBC
    variable = diffused
    boundary = '2'
    value = 0.0
  []
[]

[Materials]
  [Substance]
    type = MFEMGenericConstantMaterial
    prop_names = diffusivity
    prop_values = 1.0
  []
[]

[Kernels]
  [diff]
    type = MFEMDiffusionKernel
    variable = diffused
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
  l_tol = 1e-16
  l_max_its = 1000  
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/ParaViewDataCollection
    vtk_format = ASCII
  []
  [VisItDataCollection]
    type = MFEMVisItDataCollection
    file_base = OutputData/VisItDataCollection
  []
  [ConduitDataCollection]
    type = MFEMConduitDataCollection
    file_base = OutputData/ConduitDataCollection
    protocol = conduit_bin
  []  
[]
