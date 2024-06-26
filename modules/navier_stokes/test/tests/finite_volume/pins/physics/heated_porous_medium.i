# Fluid properties
mu = 1.1
rho = 1.1
h_fs = 100
k = 0.9
cp = 3000

# Solid properties
rho_solid = 1.1
k_solid = 13
cp_solid = 1000

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 5
    ymin = 0
    ymax = 1
    nx = 20
    ny = 10
  []
[]

[Physics]
  [NavierStokes]
    [Flow]
      [fluid]
        compressibility = 'incompressible'
        porous_medium_treatment = true

        density = 'rho'
        dynamic_viscosity = 'mu'
        porosity = 'porosity'

        initial_velocity = '1 1e-6 0'
        initial_pressure = 0.0

        # Boundary conditions
        inlet_boundaries = 'left'
        momentum_inlet_types = 'fixed-velocity'
        momentum_inlet_function = '1 0'
        wall_boundaries = 'top bottom'
        momentum_wall_types = 'noslip slip'
        outlet_boundaries = 'right'
        momentum_outlet_types = 'fixed-pressure'
        pressure_function = '0'

        mass_advection_interpolation = 'average'
        momentum_advection_interpolation = 'average'
      []
    []
    [FluidHeatTransfer]
      [fluid]
        specific_heat = 'cp'
        initial_temperature = 300

        # Reference file sets effective_conductivity that way
        # so the conductivity is multiplied by the porosity in the kernel
        effective_conductivity = false

        # See Flow for inlet and wall boundaries
        energy_inlet_types = 'fixed-temperature'
        energy_inlet_function = '300'
        energy_wall_types = 'heatflux heatflux'
        energy_wall_function = '0 0'

        ambient_convection_alpha = 'h_cv'
        ambient_temperature = 'T_solid'

        energy_advection_interpolation = 'average'
      []
    []
    [SolidHeatTransfer]
      [solid]
        block = 0

        thermal_conductivity_solid = 'k_solid'

        fixed_temperature_boundaries = 'left'
        boundary_temperatures = '300'

        # Heat source directly in the solid
        external_heat_source = '1'
        external_heat_source_coeff = 2

        ambient_convection_alpha = 'h_cv'
        ambient_convection_temperature = 'T_solid'
        verbose = true
      []
    []
  []
[]

[FunctorMaterials]
  [const_fluid]
    type = ADGenericFunctorMaterial
    prop_names = 'rho mu h_cv k cp'
    prop_values = '${rho} ${mu} ${h_fs} ${k} ${cp}'
  []
  [const_solid]
    type = ADGenericFunctorMaterial
    prop_names = 'rho_solid k_solid cp_solid'
    prop_values = '${rho_solid} ${k_solid} ${cp_solid}'
  []
[]

[AuxVariables]
  [porosity]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    initial_condition = 0.5
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  nl_rel_tol = 2e-10
  nl_abs_tol = 1e-14
  line_search = none
[]

# Some basic Postprocessors to visually examine the solution
[Postprocessors]
  [inlet-p]
    type = SideIntegralVariablePostprocessor
    variable = pressure
    boundary = 'left'
  []
  [outlet-u]
    type = SideIntegralVariablePostprocessor
    variable = superficial_vel_x
    boundary = 'right'
  []
  [outlet-Tf]
    type = SideIntegralVariablePostprocessor
    variable = T_fluid
    boundary = 'right'
  []
  [outlet-Ts]
    type = SideIntegralVariablePostprocessor
    variable = T_solid
    boundary = 'right'
  []
[]

[Outputs]
  exodus = true
  csv = true
[]
