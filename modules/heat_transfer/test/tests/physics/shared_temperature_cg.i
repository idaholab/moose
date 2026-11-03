# Two heat conduction Physics on adjacent blocks share the temperature variable
[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
  []
  [left]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    block_id = 1
    bottom_left = '0 0 0'
    top_right = '0.5 1 0'
  []
  [right]
    type = SubdomainBoundingBoxGenerator
    input = left
    block_id = 2
    bottom_left = '0.5 0 0'
    top_right = '1 1 0'
  []
[]

[Physics]
  [HeatConduction]
    [FiniteElement]
      [left]
        block = 1
        temperature_name = 'T'
        thermal_conductivity = 'k0'
        fixed_temperature_boundaries = 'left'
        boundary_temperatures = '400'
      []
      [right]
        block = 2
        temperature_name = 'T'
        thermal_conductivity = 'k0'
        fixed_temperature_boundaries = 'right'
        boundary_temperatures = '300'
      []
    []
  []
[]

[Materials]
  [mat_k]
    type = ADGenericConstantMaterial
    prop_names = 'k0'
    prop_values = '1'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Postprocessors]
  [T_mid]
    type = PointValue
    variable = T
    point = '0.5 0 0'
  []
[]

[Outputs]
  csv = true
[]
