von_karman_const = 0.41

H = 1 #halfwidth of the channel
L = 150

Re = 100

rho = 1
bulk_u = 1
mu = '${fparse rho * bulk_u * 2 * H / Re}'

[Mesh]
  [gen]
    type = CartesianMeshGenerator
    dim = 2
    dx = '${L}'
    dy = '0.667 0.333'
    ix = '200'
    iy = '10  1'
  []
[]

[Modules]
  [NavierStokesFV]
    compressibility = 'incompressible'
    turbulence_handling = 'mixing-length'

    density = ${rho}
    dynamic_viscosity = ${mu}

    initial_velocity = '1e-6 1e-6 0'
    initial_pressure = 0.0

    inlet_boundaries = 'left'
    momentum_inlet_types = 'fixed-velocity'
    momentum_inlet_function = '1 0'

    wall_boundaries = 'top bottom'
    momentum_wall_types = 'wallfunction symmetry'

    outlet_boundaries = 'right'
    momentum_outlet_types = 'fixed-pressure'
    pressure_function = '0'

    von_karman_const = ${von_karman_const}
    mixing_length_delta = 0.5
    mixing_length_walls = 'top'
    mixing_length_aux_execute_on = 'initial'

    momentum_advection_interpolation = 'upwind'
    mass_advection_interpolation = 'upwind'
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
