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
    [FiniteVolume]
      [h1]
        temperature_name = 'T'
        heat_source_var = 'Q'
        heat_source_blocks = '0'
        initial_temperature = 0

        # Thermal properties
        thermal_conductivity_functor = 'k0'
        specific_heat_functor = 5
        density_functor = 10

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
  type = Transient
  num_steps = 1
  verbose = true
[]

[AuxVariables]
  [Q]
    initial_condition = 100
  []
[]

[FunctorMaterials]
  [mat_k]
    type = ADGenericFunctorMaterial
    prop_names = 'k0'
    prop_values = '1'
  []
[]

[Outputs]
  exodus = true
[]
