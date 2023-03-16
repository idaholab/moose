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
  real_names = 'sub_val subsub0_left_val subsub1_left_val subsub0_right_val subsub1_right_val'
  real_values = '0 0 0 0 0'
[]

[MultiApps/sub]
  type = FullSolveMultiApp
  input_files = 'sub.i'
  execute_on = timestep_begin
[]

[Transfers/sub]
  type = MultiAppReporterTransfer
  from_multi_app = sub
  from_reporters = 'val/value receive/subsub0_left_val receive/subsub0_right_val receive/subsub1_left_val receive/subsub1_right_val'
  to_reporters = 'receive/sub_val receive/subsub0_left_val receive/subsub0_right_val receive/subsub1_left_val receive/subsub1_right_val'
[]

[Controls/stm]
  type = SamplerReceiver
[]
