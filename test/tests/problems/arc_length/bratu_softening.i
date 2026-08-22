!include bratu_source.i

[Problem]
  step_size := 0.3
  lambda_max := 8
  max_continuation_steps = 50
  end_on_max_continuation_steps = true
[]

[Outputs]
  file_base = bratu_softening
[]
