[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 1
[]

[Materials]
  [./time]
    type = TimeStepMaterial
    outputs = exodus
  [../]
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Executioner]
  type = Transient
  num_steps = 6

  [./TimeStepper]
    type = TimeSequenceStepper
    time_sequence = '4 8 15 16 23 42'
  [../]
[]

[Outputs]
  exodus = true
  execute_on = timestep_end
[]
