[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  ny = 1
[]

[Variables]
  [./u]
  [../]
[]

[MultiApps]
  [./sub]
    type = TransientMultiApp
    input_files = sub.i
    positions = '0 0 0'
  [../]
[]

[Transfers]
  [./sub]
    type = SamplerTransfer
    multi_app = sub
    parameters = 'BCs/left/value BCs/right/value'
    to_control = 'stochasticsub'
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 5
  dt = 0.01
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Outputs]
  execute_on = 'INITIAL TIMESTEP_END'
[]
