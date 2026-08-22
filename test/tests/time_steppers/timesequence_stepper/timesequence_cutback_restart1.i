[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  end_time = 2
  num_steps = 1
  dtmin = 0.25

  [TimeStepper]
    type = TimeSequenceStepperFailTest
    time_sequence = '0 1 2'
  []
[]

[Outputs]
  checkpoint = true
[]
