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
  [from_parent]
    type = Receiver
    default = 0
  []
  [to_inner]
    type = TimePostprocessor
  []
  [from_inner]
    type = Receiver
    default = 5
  []
  [to_parent]
    type = ParsedPostprocessor
    expression = 'from_parent + from_inner'
    pp_names = 'from_parent from_inner'
  []
[]

[MultiApps]
  [inner]
    type = TransientMultiApp
    input_files = sub_transient.i
    positions = '0 0 0'
    execute_on = TIMESTEP_BEGIN
  []
[]

[Transfers]
  [to_inner]
    type = MultiAppPostprocessorTransfer
    to_multi_app = inner
    from_postprocessor = to_inner
    to_postprocessor = from_parent
  []
  [from_inner]
    type = MultiAppPostprocessorTransfer
    from_multi_app = inner
    from_postprocessor = to_parent
    to_postprocessor = from_inner
    reduction_type = SUM
  []
[]

[Executioner]
  type = Transient
  num_steps = 5
  dt = 1
[]
