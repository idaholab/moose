[Mesh]
  type = MFEMMesh
  file = ../../mesh/square.msh
  dim = 3
[]

[Problem]
  type = MFEMProblem
  solve = false
[]

[FESpaces]
  [H1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
  []
[]

[AuxVariables]
  [u]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[Executioner]
  type = MFEMSteady
[]

[MultiApps]
  [subapp]
    type = FullSolveMultiApp
    input_files = sub.i
    execute_on = INITIAL
  []
[]

[Transfers]
  [from_sub]
    type = MultiAppMFEMCopyTransfer
    source_variable = u
    variable = u
    from_multi_app = subapp
  []
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/Diffusion
    vtk_format = ASCII
  []
[]
