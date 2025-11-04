# This version uses VolumeJunction1Phase

!include base_params.i

radius = ${fparse 0.5 * diam}
volume = ${fparse 4/3 * pi * radius^3}

!include base.i

[GlobalParams]
  initial_vel_x = 0
  initial_vel_y = 0
  initial_vel_z = 0

  scaling_factor_rhouV = 1e-2
  scaling_factor_rhovV = 1e-2
  scaling_factor_rhowV = 1e-2
  scaling_factor_rhoEV = 1e-5

  apply_velocity_scaling = false
[]

[Components]
  [junction_heated_top]
    type = VolumeJunction1Phase
    position = '0 0 1'
    volume = ${volume}
    connections = 'heated_pipe:out top_pipe:in'
  []
  [junction_top_cooled]
    type = VolumeJunction1Phase
    position = '1 0 1'
    volume = ${volume}
    connections = 'top_pipe:out cooled_pipe:in'
  []
  [junction_cooled_bottom]
    type = VolumeJunction1Phase
    position = '0 0 1'
    volume = ${volume}
    connections = 'cooled_pipe:out bottom_pipe:in'
  []
  [junction_bottom_heated]
    type = VolumeJunction1Phase
    position = '0 0 0'
    volume = ${volume}
    connections = 'bottom_pipe:out heated_pipe:in'
  []
[]
