[Mesh]
  type = MFEMMesh
  file = ../../mesh/square.msh
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
  [recv]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[Executioner]
  type = MFEMSteady
[]

[VectorPostprocessors]
  [line_sample]
    type = MFEMLineValueSampler
    variable = 'recv'
    start_point = '0 0 0'
    end_point = '1 1 0.1'
    num_points = 101
  []
[]

[Outputs]
  [CSV]
    type = CSV
    execute_on = 'timestep_end'
    file_base = OutputData/DiffusionRecvApp/recv
  []
[]
