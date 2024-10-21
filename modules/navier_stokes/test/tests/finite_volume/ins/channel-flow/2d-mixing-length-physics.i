Re = 1e4
von_karman_const = 0.2

D = 1
rho = 1
bulk_u = 1
mu = '${fparse rho * bulk_u * D / Re}'

advected_interp_method = 'upwind'

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 5
    ymin = 0
    ymax = '${fparse 0.5 * D}'
    nx = 20
    ny = 10
    bias_y = '${fparse 1 / 1.2}'
  []
[]

[Physics]
  [NavierStokes]
    [Flow]
      [flow]
        compressibility = 'incompressible'

        density = ${rho}
        dynamic_viscosity = ${mu}

        initial_velocity = '1 1 0'
        initial_pressure = 0.0

        inlet_boundaries = 'left'
        momentum_inlet_types = 'fixed-velocity'
        momentum_inlet_function = '1 0'

        wall_boundaries = 'top bottom'
        momentum_wall_types = 'noslip symmetry'

        outlet_boundaries = 'right'
        momentum_outlet_types = 'fixed-pressure'
        pressure_function = '0'

        momentum_advection_interpolation = ${advected_interp_method}
        mass_advection_interpolation = ${advected_interp_method}
      []
    []
    [ScalarTransport]
      [scalars]
        add_scalar_equation = true
        passive_scalar_names = 'scalar'
        passive_scalar_source = 0.1
        passive_scalar_inlet_types = 'fixed-value'
        passive_scalar_inlet_function = '1'
        passive_scalar_advection_interpolation = ${advected_interp_method}
      []
    []
  []
[]
# This separation is introduced for documentation purposes.
# Both could be nested under Physics/NavierStokes
[Physics/NavierStokes]
  [Turbulence]
    [mixing-length]
      turbulence_handling = 'mixing-length'
      coupled_flow_physics = flow
      scalar_transport_physics = scalars

      passive_scalar_schmidt_number = 1.0

      von_karman_const = ${von_karman_const}
      mixing_length_delta = 1e9
      mixing_length_walls = 'top bottom'
      mixing_length_aux_execute_on = 'initial'
    []
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
