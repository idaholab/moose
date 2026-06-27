[Mesh]
  type = MFEMMesh
  file = ../mesh/square.e
[]

[Problem]
  type = MFEMProblem
  solve = false
[]

[Postprocessors]
  [coupling_its]
    type = NumFixedPointIterations
    execute_on = 'initial timestep_end'
  []
  # The parent sends a value to the sub-app, the sub-app increments it, and the result is fed back
  # into the parent on the next fixed-point iteration.
  [to_sub]
    type = ParsedPostprocessor
    expression = 'from_sub + 1'
    pp_names = 'from_sub'
    execute_on = 'timestep_begin'
  []
  [from_sub]
    type = Receiver
    default = 0
  []
[]

[MultiApps]
  [sub]
    type = FullSolveMultiApp
    input_files = sub_fixed_point_steady.i
    execute_on = timestep_begin
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
  type = MFEMSteady
  fixed_point_min_its = 3
  fixed_point_max_its = 3
  disable_fixed_point_residual_norm_check = true
  accept_on_max_fixed_point_iteration = true
[]

[Outputs]
  csv = true
[]
