# Test that the initial conditions read from the exodus file are correct

!include base.i

[GlobalParams]
  initial_from_file = 'steady_state_out.e'
  initial_from_file_weighting_type = largest_element_id
[]

[Executioner]
  num_steps = 0
[]

[Outputs]
  execute_on = 'initial'
[]
