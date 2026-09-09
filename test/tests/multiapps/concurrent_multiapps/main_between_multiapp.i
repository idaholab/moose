# Check that the transfers and app executions occur in the expected order
# App1 execution then transfers from app1 to app2, then app2 execution

[Problem]
  solve = false
  verbose_multiapps = true
  num_concurrent_multiapps = 2
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[MultiApps]
  [ma1]
    type = TransientMultiApp
    input_files = sub_between_diffusion1.i
    max_procs_per_app = 2
    output_in_position = true
  []
[]

[MultiApps]
  [ma2]
    type = TransientMultiApp
    input_files = sub_between_diffusion2.i
    output_in_position = true
  []
[]

[Transfers]
  [pp_1_to_2]
    type = MultiAppPostprocessorTransfer
    from_multi_app = ma1
    to_multi_app = ma2
    from_postprocessor = 'time_app1'
    to_postprocessor = 'time_in_app1'
    execute_after_from_multiapp = true
  []
  [pp_2_to_1]
    type = MultiAppPostprocessorTransfer
    from_multi_app = ma2
    to_multi_app = ma1
    from_postprocessor = 'time_app2'
    to_postprocessor = 'time_in_app2'
  []
[]

[Positions]
  [app1_locs]
    type = InputPositions
    positions = '0 0 0
                 0 1.01 0'
  []
  [app2_locs]
    type = InputPositions
    positions = '-0.7 -0.45 0
                 0.7 0.3 0
                 -0.5 0.5 0'
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]
