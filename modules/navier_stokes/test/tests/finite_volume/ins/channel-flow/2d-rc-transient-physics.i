# Fluid properties
mu = 1.1
rho = 1.1
cp = 1.1
k = 1e-3

# Operating conditions
u_inlet = 1
T_inlet = 200
T_solid = 190
p_outlet = 10
h_fs = 0.01

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 5
    ymin = -1
    ymax = 1
    nx = 50
    ny = 20
  []
[]

[Physics]
  [NavierStokes]
    [Flow]
      [flow]
        compressibility = 'incompressible'

        density = 'rho'
        dynamic_viscosity = 'mu'

        initial_velocity = '${u_inlet} 1e-12 0'
        initial_pressure = 0.0

        inlet_boundaries = 'left'
        momentum_inlet_types = 'fixed-velocity'
        momentum_inlet_function = '${u_inlet} 0'
        wall_boundaries = 'bottom top'
        momentum_wall_types = 'symmetry noslip'

        outlet_boundaries = 'right'
        momentum_outlet_types = 'fixed-pressure-zero-gradient'
        pressure_function = '${p_outlet}'

        mass_advection_interpolation = 'average'
        momentum_advection_interpolation = 'average'
      []
    []
  []
[]
# This separation is introduced for documentation purposes.
# Both Physics could be nested under Physics/NavierStokes
[Physics/NavierStokes]
  [FluidHeatTransfer]
    [heat]
      thermal_conductivity = 'k'
      specific_heat = 'cp'

      fluid_temperature_variable = 'T_fluid'
      initial_temperature = '${T_inlet}'
      energy_inlet_types = 'heatflux'
      energy_inlet_functors = '${fparse u_inlet * rho * cp * T_inlet}'

      energy_wall_types = 'heatflux heatflux'
      energy_wall_functors = '0 0'

      ambient_convection_alpha = 'h_cv'
      ambient_temperature = 'T_solid'

      energy_advection_interpolation = 'average'
    []
  []
[]

[FunctorMaterials]
  [constants]
    type = ADGenericFunctorMaterial
    prop_names = 'h_cv T_solid rho mu cp k'
    prop_values = '${h_fs} ${T_solid} ${rho} ${mu} ${cp} ${k}'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  line_search = 'none'
  nl_rel_tol = 7e-13
  dt = 0.4
  end_time = 0.8
[]

[Outputs]
  exodus = true
  csv = true
[]
