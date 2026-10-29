# Two finite volume heat conduction Physics on blocks that do not touch share the temperature
# variable. Block 2, between them, has no Physics.
[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
  []
  [middle]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    block_id = 2
    bottom_left = '0.4 0 0'
    top_right = '0.6 1 0'
  []
  [right]
    type = SubdomainBoundingBoxGenerator
    input = middle
    block_id = 3
    bottom_left = '0.6 0 0'
    top_right = '1 1 0'
  []
[]

[Physics]
  [HeatConduction]
    [FiniteVolume]
      [left]
        block = 0
        temperature_name = 'T'
        thermal_conductivity_functor = 'k0'
        fixed_temperature_boundaries = 'left'
        boundary_temperatures = '400'
      []
      [right]
        block = 3
        temperature_name = 'T'
        thermal_conductivity_functor = 'k0'
        fixed_temperature_boundaries = 'right'
        boundary_temperatures = '300'
      []
    []
  []
[]

[FunctorMaterials]
  [mat_k]
    type = ADGenericFunctorMaterial
    prop_names = 'k0'
    prop_values = '1'
  []
[]

[Problem]
  # Block 2 has no Physics, hence no kernels
  kernel_coverage_check = false
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Postprocessors]
  [T_left]
    type = ElementAverageValue
    variable = T
    block = 0
  []
  [T_right]
    type = ElementAverageValue
    variable = T
    block = 3
  []
[]

[Outputs]
  csv = true
[]
