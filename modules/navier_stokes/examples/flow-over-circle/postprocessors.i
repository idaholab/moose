
[Postprocessors]
  [Re]
    type = ParsedPostprocessor
    expression = 'rho * U * D / mu'
    constant_names = 'rho U D mu'
    constant_expressions = '${rho} ${fparse 2/3*inlet_velocity} ${fparse 2*circle_radius} ${mu}'
  []
  [point_vel_x]
    type = PointValue
    point = '${fparse (x_max-x_min)/2} ${fparse (y_max-y_min)/2} 0'
    variable = 'vel_x'
  []
  [point_vel_y]
    type = PointValue
    point = '${fparse (x_max-x_min)/2} ${fparse (y_max-y_min)/2} 0'
    variable = 'vel_y'
  []
  [drag_force]
    type = IntegralDirectedSurfaceForce
    vel_x = vel_x
    vel_y = vel_y
    mu = ${mu}
    pressure = pressure
    principal_direction = '1 0 0'
    boundary = 'circle'
    outputs = none
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [drag_coeff]
    type = ParsedPostprocessor
    expression = '2*drag_force/rho/(avgvel*avgvel)/D'
    constant_names = 'rho avgvel D'
    constant_expressions = '${rho} ${fparse 2/3*inlet_velocity} ${fparse 2*circle_radius}'
    pp_names = 'drag_force'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [lift_force]
    type = IntegralDirectedSurfaceForce
    vel_x = vel_x
    vel_y = vel_y
    mu = ${mu}
    pressure = pressure
    principal_direction = '0 1 0'
    boundary = 'circle'
    outputs = none
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [lift_coeff]
    type = ParsedPostprocessor
    expression = '2*lift_force/rho/(avgvel*avgvel)/D'
    constant_names = 'rho avgvel D'
    constant_expressions = '${rho} ${fparse 2/3*inlet_velocity} ${fparse 2*circle_radius}'
    pp_names = 'lift_force'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]
