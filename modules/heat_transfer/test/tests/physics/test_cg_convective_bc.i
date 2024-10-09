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
        heat_source_functor = '100'
        heat_source_blocks = '0'
        initial_temperature = 0

        # Thermal properties
        thermal_conductivity = 'k0'

        # Boundary conditions
        fixed_convection_boundaries = 'left right'
        fixed_convection_T_fluid = '0 500'
        fixed_convection_htc = '1 2'
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
