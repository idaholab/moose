[Mesh]
  type = MFEMMesh
  file = gold/mug.e
  dim = 3
[]

[Problem]
  type = MFEMProblem
  solve = false
[]

[FESpaces]
  [H1FESpace]
    type = MFEMFESpace
    fec_type = H1
    fec_order = FIRST
  []
[]

[AuxVariables]
  [recv]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[MultiApps]
  [./subapp]
    type = FullSolveMultiApp
    input_files = sub.i
    execute_on = FINAL
  [../]
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Transfers]
    [./to_sub]
        type = MultiAppMFEMCopyTransfer
        source_variable = send
        variable = recv
        from_multi_app = subapp
    [../]
[]


