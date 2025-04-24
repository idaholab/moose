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

#[AuxVariables]
#  [recv]
#    type = MFEMVariable
#    fespace = H1FESpace
#  []
#[]

[MultiApps]
  [recv_app]
    type = FullSolveMultiApp
    input_files = sub_recv.i
    execute_on = FINAL
  []
  [send_app]
    type = FullSolveMultiApp
    input_files = sub_send.i
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
        source_variable = send
        variable = recv
        from_multi_app = send_app
        to_multi_app = recv_app
    []
[]
