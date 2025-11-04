[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = 10
    dy = 10
  []
[]

[Physics]
  [HeatConduction]
    [FiniteElement]
      [h1]
        temperature_name = 'T'
        heat_source_var = 'Q'
        heat_source_blocks = '0'
        initial_temperature = 0

        # Thermal properties
        thermal_conductivity = 'k0'

        # Boundary conditions
        heat_flux_boundaries = 'left right'
        boundary_heat_fluxes = '0 500'
        insulated_boundaries = 'top'
        fixed_temperature_boundaries = 'bottom'
        boundary_temperatures = '300'
      []
    []
  []
[]

[Executioner]
  type = Steady
  verbose = true
[]

[AuxVariables]
  [Q]
    initial_condition = 100
  []
[]

[Materials]
  [mat_k]
    type = ADGenericConstantMaterial
    prop_names = 'k0'
    prop_values = '1'
  []
[]

[Outputs]
  exodus = true
[]
