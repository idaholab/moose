mu = 1.1
rho = 1.1

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 5
    ymin = -1
    ymax = 1
    nx = 50
    ny = 10
  []
[]

[Modules]
  [NavierStokesFV]
    compressibility = 'incompressible'

    density = 'rho'
    dynamic_viscosity = 'mu'

    initial_velocity = '1 1 0'
    initial_pressure = 0.0

    inlet_boundaries = 'left'
    momentum_inlet_types = 'fixed-velocity'
    momentum_inlet_function = '1 0'
    wall_boundaries = 'top bottom'
    momentum_wall_types = 'noslip noslip'
    outlet_boundaries = 'right'
    momentum_outlet_types = 'fixed-pressure'
    pressure_function = '0'

    friction_types = 'darcy'
    friction_coeffs = '25'

    mass_advection_interpolation = 'average'
    momentum_advection_interpolation = 'average'
  []
[]

[Materials]
  [const]
    type = ADGenericFunctorMaterial
    prop_names = 'rho mu'
    prop_values = '${rho} ${mu}'
  []
  [speed_material]
    type = PINSFVSpeedFunctorMaterial
    superficial_vel_x = vel_x
    superficial_vel_y = vel_y
    porosity = 1
  []
  [Re_material]
    type = ReynoldsNumberFunctorMaterial
    speed = speed
    characteristic_length = 2
    rho = ${rho}
    mu = ${mu}
  []
  [exponential_friction_coefficient]
    type = ExponentialFrictionMaterial
    friction_factor_name = 'friction_coefficient'
    Re = Re
    c1 = 0.25
    c2 = 0.55
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
