[Mesh]
  active = 'cmg'
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = 10
    dy = 10
  []
  [fmg_restart]
    type = FileMeshGenerator
    file = user_ics.e
    use_for_exodus_restart = true
  []
[]

[Debug]
  show_actions=true
[]

[Physics]
  [HeatConduction]
    [FiniteVolume]
      [h1]
        temperature_name = 'T'

        # Thermal properties
        thermal_conductivity_functor = 'k0'
        specific_heat = 5
        density = 10

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

[Problem]
  solve = false
[]

[FunctorMaterials]
  [mat_k]
    type = ADGenericFunctorMaterial
    prop_names = 'k0'
    prop_values = '1'
  []
[]

[Outputs]
  # Used to set up a restart from checkpoint
  checkpoint = true
  # Used to set up a restart from exodus file
  [exodus]
    type = Exodus
    execute_on = TIMESTEP_END
  []
  # Used to check results
  csv = true
  execute_on = INITIAL
[]

[Postprocessors]
  [min_T]
    type = ElementExtremeValue
    variable = 'T'
    value_type = 'min'
    execute_on = 'INITIAL'
  []
  [max_T]
    type = ElementExtremeValue
    variable = 'T'
    value_type = 'max'
    execute_on = 'INITIAL'
  []
[]
