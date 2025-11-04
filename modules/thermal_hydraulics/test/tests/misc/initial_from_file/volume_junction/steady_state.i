!include base.i

[GlobalParams]
  initial_T = 500
  initial_p = 6.e6
  initial_vel = 0
[]

[Components]
  [junction]
    initial_vel_x = 0
    initial_vel_y = 0
    initial_vel_z = 0
  []
[]

[Executioner]
  num_steps = 5
[]

[Outputs]
  execute_on = 'initial final'
[]
