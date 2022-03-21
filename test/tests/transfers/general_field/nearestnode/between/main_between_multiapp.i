[Problem]
  solve = false
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[MultiApps/ma1]
  type = TransientMultiApp
  input_files = sub_between_diffusion1.i
  max_procs_per_app = 1
[]

[MultiApps/ma2]
  type = TransientMultiApp
  input_files = sub_between_diffusion2.i
  max_procs_per_app = 2
[]

[Transfers/from_ma1_to_ma2]
  type = MultiAppGeneralFieldNearestNodeTransfer
  from_multi_app = ma1
  to_multi_app = ma2
  source_variable = u
  variable = transferred_u
[]

[Transfers/from_ma2_to_ma1]
  type = MultiAppGeneralFieldNearestNodeTransfer
  from_multi_app = ma2
  to_multi_app = ma1
  source_variable = u
  variable = transferred_u
[]

[Executioner]
  type = Transient
  num_steps = 1
[]
