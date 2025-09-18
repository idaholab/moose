[Mesh/fuel_pin]
  type = FileMeshGenerator
  file = fuel_pin.e
[]

[Physics/HeatConduction/FiniteElement/heat_conduction]
  # Apply heat conduction to the fuel and the cladding
  block = 'fuel clad'

  # Fix the outer boundary to a value of 300
  boundary_temperatures = 300 # [K]
  fixed_temperature_boundaries = outer

  # Insulate the inner boundary (zero heat flux)
  insulated_boundaries = inner

  # Name of the thermal conductivity material property
  thermal_conductivity = k

  # Apply a constant heat source to the fuel
  heat_source_blocks = fuel
  heat_source_functor = 1e8 # [W/m^2]

  use_automatic_differentiation = false
[]

[Variables]
  [T] # [K]
  []
[]

[Materials]
  [k_fuel]
    type = GenericConstantMaterial
    prop_names = k
    prop_values = 2 # [W/m*K]
    block = fuel
  []
  [k_clad]
    type = GenericConstantMaterial
    prop_names = k
    prop_values = 10 # [W/m*K]
    block = clad
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
