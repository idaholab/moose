# Test that the initial conditions read from the exodus file are correct

!include base.i

[GlobalParams]
  initial_from_file = 'steady_state_out.e'
[]

[Executioner]
  num_steps = 0
[]

[Outputs]
  execute_on = 'initial'
[]
