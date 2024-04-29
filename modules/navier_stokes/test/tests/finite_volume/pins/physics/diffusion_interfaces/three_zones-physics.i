D0 = 1
D1 = 2
D2 = 6

[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 1
    dx = '1.5 3 2'
    ix = '3 3 4'
    subdomain_id = '0 1 2'
  []
[]

[Physics]
  [NavierStokes]
    [SolidHeatTransfer]
      [solid]
        block = '0 1 2'
        thermal_conductivity_solid = '${D0} ${D1} ${D2}'
        thermal_conductivity_blocks = '0; 1; 2'
        fixed_temperature_boundaries = 'right'
        boundary_temperatures = '1'

        external_heat_source = '1'
        external_heat_source_blocks = '1'

        # not needed for steady state
        use_external_enthalpy_material = true

        # to match reference results
        initial_temperature = 0
        porosity = 1
        verbose = true
      []
    []
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]

[VectorPostprocessors]
  [all_values]
    type = ElementValueSampler
    variable = T_solid
    sort_by = 'x'
  []
[]
