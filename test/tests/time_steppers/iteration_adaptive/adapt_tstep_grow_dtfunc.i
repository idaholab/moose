[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [dt]
    type = TimeDerivative
    variable = u
  []
[]

[Executioner]
  type = Transient
  end_time = 20.0
  verbose = true

  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1.0
    optimal_iterations = 10
    time_t = '0.0 5.0'
    time_dt = '1.0 5.0'
  []
[]

[Postprocessors]
  [_dt]
    type = TimestepSize
  []
[]

[Outputs]
  csv = true
  checkpoint = true
  [ckp]
    type = Checkpoint
    num_files = 3
  []
[]
