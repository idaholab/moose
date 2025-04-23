[Mesh]
  type = MFEMMesh
  file = ../../../../../../unit/data/square.msh
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

[MultiApps]
  [subapp]
    type = FullSolveMultiApp
    input_files = sub.i
    execute_on = INITIAL
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Transfers]
    [to_sub]
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

