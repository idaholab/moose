mu = 1
rho = 1
k = 1e-3
diff = 1e-3
cp = 1

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 10
    ymin = -1
    ymax = 1
    nx = 100
    ny = 20
  []
[]

[Variables]
  inactive = 'vel_x vel_y pressure T_fluid scalar'
  [vel_x]
    type = 'INSFVVelocityVariable'
    initial_condition = 1
  []
  [vel_y]
    type = 'INSFVVelocityVariable'
    initial_condition = 1
  []
  [pressure]
    type = 'INSFVPressureVariable'
    initial_condition = 0
  []
  [T_fluid]
    type = 'INSFVEnergyVariable'
    initial_condition = 0
  []
  [scalar]
    type = MooseVariableFVReal
    initial_condition = 0
  []
[]

[Physics]
  [NavierStokes]
    [Flow]
      [flow]
        compressibility = 'incompressible'

        density = ${rho}
        dynamic_viscosity = ${mu}

        inlet_boundaries = 'left'
        momentum_inlet_types = 'fixed-velocity'
        momentum_inlet_function = '1 0'

        wall_boundaries = 'top bottom'
        momentum_wall_types = 'noslip noslip'

        outlet_boundaries = 'right'
        momentum_outlet_types = 'fixed-pressure'
        pressure_function = '0'

        mass_advection_interpolation = 'average'
        momentum_advection_interpolation = 'average'
      []
    []
    [FluidHeatTransfer]
      [heat]
        thermal_conductivity = ${k}
        specific_heat = ${cp}

        energy_inlet_types = 'fixed-temperature'
        energy_inlet_function = '1'

        energy_wall_types = 'heatflux heatflux'
        energy_wall_function = '0 0'

        energy_advection_interpolation = 'average'
      []
    []
  []
[]
# This separation is introduced for documentation purposes.
# Both could be nested under Physics/NavierStokes
[Physics/NavierStokes]
  [ScalarTransport]
    [heat]
      passive_scalar_names = 'scalar'

      passive_scalar_diffusivity = ${diff}
      passive_scalar_source = 0.1
      passive_scalar_coupled_source = U
      passive_scalar_coupled_source_coeff = 0.1

      passive_scalar_inlet_types = 'fixed-value'
      passive_scalar_inlet_function = '1'

      passive_scalar_advection_interpolation = 'average'
    []
  []
[]

[AuxVariables]
  [U]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
[]

[AuxKernels]
  [mag]
    type = VectorMagnitudeAux
    variable = U
    x = vel_x
    y = vel_y
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
  csv = true
[]
