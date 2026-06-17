[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[Variables]
  [u]
  []
[]

[Problem]
  solve = false
[]

[Postprocessors]
  [to_sub]
    type = TimePostprocessor
  []
  [from_sub]
    type = Receiver
    default = 5
  []
[]

[MultiApps]
  [sub]
    type = FullSolveMultiApp
    input_files = sub_transient.i
    positions = '0 0 0'
    execute_on = TIMESTEP_END
  []
[]

[Transfers]
  [to_sub]
    type = MultiAppPostprocessorTransfer
    to_multi_app = sub
    from_postprocessor = to_sub
    to_postprocessor = from_parent
  []
  [from_sub]
    type = MultiAppPostprocessorTransfer
    from_multi_app = sub
    from_postprocessor = to_parent
    to_postprocessor = from_sub
    reduction_type = SUM
  []
[]

[Executioner]
  type = Transient
  num_steps = 5
  dt = 10
[]

[Outputs]
  csv = true
[]
