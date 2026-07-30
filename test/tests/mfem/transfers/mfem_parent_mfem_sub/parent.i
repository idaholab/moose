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
  active = 'copy_from_sub'
  [copy_from_sub]
    type = MultiAppMFEMCopyTransfer
    source_variables = u
    variables = u
    from_multi_app = subapp
  []
  [general_transfer_from_sub]
    type = MultiAppMFEMShapeEvaluationTransfer
    source_variables = u
    variables = u
    from_multi_app = subapp
  []
[]

[VectorPostprocessors]
  [line_sample]
    type = MFEMLineValueSampler
    variable = 'u'
    start_point = '0 0 0'
    end_point = '1 1 0.1'
    num_points = 101
  []
[]

[Outputs]
  [CSV]
    type = CSV
    execute_on = 'timestep_end'
    file_base = OutputData/Diffusion/parent
  []
[]
