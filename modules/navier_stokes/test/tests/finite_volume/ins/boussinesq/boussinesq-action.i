mu = 1
rho = 1
k = 1
cp = 1
alpha = 1
rayleigh = 1e3
hot_temp = ${rayleigh}
temp_ref = '${fparse hot_temp / 2.}'

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 32
    ny = 32
  []
[]

[Physics]
  [NavierStokes]
    [Flow/flow]
      compressibility = 'incompressible'
      porous_medium_treatment = false
      boussinesq_approximation = true

      initial_pressure = 0.0

      density = ${rho}
      dynamic_viscosity = ${mu}
      thermal_expansion = ${alpha}
      ref_temperature = ${temp_ref}
      gravity = '0 -1 0'

      inlet_boundaries = 'top'
      momentum_inlet_types = 'fixed-velocity'
      momentum_inlet_functors = 'lid_function 0'

      wall_boundaries = 'left right bottom'
      momentum_wall_types = 'noslip noslip noslip'

      pin_pressure = true
      pinned_pressure_type = average
      pinned_pressure_value = 0

      momentum_advection_interpolation = 'upwind'
      mass_advection_interpolation = 'upwind'
    []
    [FluidHeatTransfer/energy]
      thermal_conductivity = ${k}
      specific_heat = ${cp}

      initial_temperature = 0.0

      energy_inlet_types = 'heatflux'
      energy_inlet_function = '0'

      energy_wall_types = 'fixed-temperature fixed-temperature heatflux'
      energy_wall_functors = '${hot_temp} 0 0'

      energy_advection_interpolation = 'upwind'
      energy_scaling = 1e-4
    []
  []
[]

[Functions]
  [lid_function]
    type = ParsedFunction
    expression = '4*x*(1-x)'
  []
  [rho_error]
    type = ParsedFunction
    expression = '4*x*(1-x)'
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
