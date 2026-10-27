rho = 1
mu = 1

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system'
[]

[Physics]
  [NavierStokes]
    [FlowSegregated/flow]
      velocity_variable = 'vel_x vel_y'
      pressure_variable = pressure

      initial_velocity = '0.5 0 0'
      initial_pressure = 0

      density = ${rho}
      dynamic_viscosity = ${mu}

      inlet_boundaries = left
      momentum_inlet_types = fixed-velocity
      momentum_inlet_functors = '1 0'

      wall_boundaries = 'top bottom'
      momentum_wall_types = 'noslip noslip'

      outlet_boundaries = right
      momentum_outlet_types = fixed-pressure
      pressure_functors = 0

      orthogonality_correction = false
      momentum_two_term_bc_expansion = false
      pressure_two_term_bc_expansion = false
    []
  []
[]

