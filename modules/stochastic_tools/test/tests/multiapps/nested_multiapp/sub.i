[Mesh/gmg]
  type = GeneratedMeshGenerator
  dim = 1
  nx = 10
  xmax = 1
[]

[Variables/u]
[]

[Kernels/diff]
  type = Diffusion
  variable = u
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 10
  []
[]

[Postprocessors/val]
  type = PointValue
  variable = u
  point = '0 0 0'
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Reporters/receive]
  type = ConstantReporter
  real_names = 'subsub0_left_val subsub1_left_val subsub0_right_val subsub1_right_val'
  real_values = '0 0 0 0'
[]

[MultiApps/subsub]
  type = FullSolveMultiApp
  input_files = 'subsub.i'
  positions = '0 0 0 1 0 0'
  execute_on = timestep_begin
[]

[Transfers]
  [subsub0]
    type = MultiAppReporterTransfer
    from_multi_app = subsub
    from_reporters = 'lval/value rval/value'
    to_reporters = 'receive/subsub0_left_val receive/subsub0_right_val'
    subapp_index = 0
  []
  [subsub1]
    type = MultiAppReporterTransfer
    from_multi_app = subsub
    from_reporters = 'lval/value rval/value'
    to_reporters = 'receive/subsub1_left_val receive/subsub1_right_val'
    subapp_index = 1
  []
[]

[Controls/stm]
  type = SamplerReceiver
[]
