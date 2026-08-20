!include bratu_source.i

[Problem]
  step_size := 0.3
  lambda_max := 8
  # the fold sits near 6.8, so a 'lambda_max' of 8 is never reached and the budget ends the path
  max_continuation_steps = 50
  end_on_max_continuation_steps = true
[]

[Outputs]
  file_base = bratu_softening
[]
