# This test is designed to check for energy conservation
# in separated channels. The three inlet temperatures should be
# preserved at the outlets.

rho=1.1
mu=1e-4
k=2.1
cp=5.5

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '0.25 1.0 0.25'
    dy = '0.25 0.25 0.25'
    ix = '4 20 4'
    iy = '5 5 5'
    subdomain_id = '1 2 5 1 3 5 1 4 5'
  []
  [separator-1]
    type = SideSetsBetweenSubdomainsGenerator
    input = mesh
    primary_block = '2'
    paired_block = '3'
    new_boundary = 'separator-1'
  []
  [separator-2]
    type = SideSetsBetweenSubdomainsGenerator
    input = separator-1
    primary_block = '3'
    paired_block = '4'
    new_boundary = 'separator-2'
  []
  [jump-1]
    type = SideSetsBetweenSubdomainsGenerator
    input = separator-2
    primary_block = '1'
    paired_block = '2'
    new_boundary = jump-1
  []
  [jump-2]
    type = SideSetsBetweenSubdomainsGenerator
    input = jump-1
    primary_block = '1'
    paired_block = '3'
    new_boundary = jump-2
  []
  [jump-3]
    type = SideSetsBetweenSubdomainsGenerator
    input = jump-2
    primary_block = '1'
    paired_block = '4'
    new_boundary = jump-3
  []
  [outlet-1]
    type = SideSetsBetweenSubdomainsGenerator
    input = jump-3
    primary_block = '2'
    paired_block = '5'
    new_boundary = outlet-1
  []
  [outlet-2]
    type = SideSetsBetweenSubdomainsGenerator
    input = outlet-1
    primary_block = '3'
    paired_block = '5'
    new_boundary = outlet-2
  []
  [outlet-3]
    type = SideSetsBetweenSubdomainsGenerator
    input = outlet-2
    primary_block = '4'
    paired_block = '5'
    new_boundary = outlet-3
  []
[]

[Physics]
  [NavierStokes]
    [Flow]
      [flow]
        compressibility = 'incompressible'
        porous_medium_treatment = true

        # Material property parameters
        density = ${rho}
        dynamic_viscosity = ${mu}

        # Porous medium parameters
        porosity = porosity
        porosity_interface_pressure_treatment = 'bernoulli'
        pressure_drop_sidesets = 'jump-1 jump-2 jump-3 outlet-1 outlet-2 outlet-3'
        pressure_drop_form_factors = '0.1 0.2 0.3 0.1 0.2 0.3'
        friction_types = 'forchheimer'
        friction_coeffs = 'Forchheimer_coefficient'

        # Initial conditions
        initial_velocity = '0.1 0 0'
        initial_pressure = 0.0

        # Boundary conditions
        inlet_boundaries = 'left'
        momentum_inlet_types = 'fixed-velocity'
        momentum_inlet_function = '0.1 0'

        wall_boundaries = 'top bottom'
        momentum_wall_types = 'slip slip'

        outlet_boundaries = 'right'
        momentum_outlet_types = 'fixed-pressure'
        pressure_function = '0.4'

        hydraulic_separator_sidesets = 'separator-1 separator-2'
      []
    []
    [FluidHeatTransfer]
      [heat]
        # Material properties
        thermal_conductivity = ${k}
        specific_heat = ${cp}

        # Initial conditions
        initial_temperature = 300.0

        # Boundary conditions
        energy_inlet_types = 'fixed-temperature'
        energy_inlet_function = 300.0

        energy_wall_types = 'heatflux heatflux'
        energy_wall_function = '0 0'

        # Heat source
        external_heat_source = heating
      []
    []
  []
[]

[Functions]
  [heating]
    type = ParsedFunction
    expression = 'if(x>0.25 & x<1.25, if(y<0.25, 10, if(y<0.5, 20, 30)), 0)'
  []
[]

[FunctorMaterials]
  [porosity]
    type = ADPiecewiseByBlockFunctorMaterial
    prop_name = porosity
    subdomain_to_prop_value = '1 0.8
                               2 0.7
                               3 0.6
                               4 0.5
                               5 0.8'
  []
  [fc-1]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'Forchheimer_coefficient'
    prop_values = '1.0 1.0 1.0'
    block = '1 5'
  []
  [fc-2]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'Forchheimer_coefficient'
    prop_values = '3.0 3.0 3.0'
    block = 2
  []
  [fc-3]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'Forchheimer_coefficient'
    prop_values = '1.5 1.5 1.5'
    block = 3
  []
  [fc-4]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'Forchheimer_coefficient'
    prop_values = '0.75 0.75 0.75'
    block = 4
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = ' lu       NONZERO               1e-10'
  line_search = 'none'
  nl_rel_tol = 1e-10
[]

[Postprocessors]
  [outlet_T1]
    type = SideAverageValue
    variable = 'T_fluid'
    boundary = 'right'
  []
[]

[Outputs]
  csv = true
  execute_on = final
[]
