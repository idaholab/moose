[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]
[Variables]
  [./u]
  [../]
[]
[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[Reporters]
  [capsule_01]
    type = ConstantReporter
    real_vector_vector_names = 'x temp nonsense'
    real_vector_vector_values = '0 0 0 0;0 0 0 0 | 0 0 0 0; 0 0 0 0 | 1 2 ; 3 4 5; 6 7 8 9 0; 10 11 12 13 14 15'
    outputs = json
  []
[]
[Transfers]
  [vpp_to_vpp1]
      type = MultiAppReporterTransfer
      from_multi_app = capsule1
      to_reporters = 'capsule_01/x capsule_01/temp'
      from_reporters = 'temp/x temp/temperature'
      distribute_reporter_vector = true
  []
[]

[MultiApps]
  [capsule1]
    type = FullSolveMultiApp
    execute_on = initial
    input_files = 'vpp_general.i vpp_specific_01.i vpp_specific_02.i'
    positions = '0 0 0 0 0 0 0 0 0'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'

[]
[Outputs]
  [csv]
    type = CSV
    execute_on = FINAL
    execute_vector_postprocessors_on = FINAL
  []
  json = true
[]
