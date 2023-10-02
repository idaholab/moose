[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = 10
    dy = 10
  []
[]

[Physics]
  [hc]
    type = HeatConductionPhysics
    temperature_name = 'T'
    heat_source_var = 'Q'

    # Thermal properties
    thermal_conductivity = 'k'

    # Boundary conditions
    heat_flux_boundaries = 'left right'
    boundary_heat_fluxes = '0 500'
    insulated_boundaries = 'top'
    fixed_temperature_boundaries = 'bottom'
    boundary_temperatures = '300'

    # Discretization
    # discretization = 'cgfe'
    [Discretization]
      type = ContinuousGalerkin
    []
  []
[]

[Executioner]
  type = Steady
[]

[Functions]
  [Q]
    type = ParsedFunction
    expression = 'x + y'
  []
[]

[Materials]
  [mat_k]
    type = ADGenericConstantMaterial
    prop_names = 'k'
    prop_values = '1'
  []
[]

[Debug]
  show_parser = true
[]

[Outputs]
  exodus = true
[]
