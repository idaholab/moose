[Mesh/fuel_pin]
  type = FileMeshGenerator
  file = fuel_pin.e
[]

[Physics/HeatConduction/FiniteElement/heat_conduction]
  # Solve heat conduction on fuel and cladding
  block = 'fuel clad'

  # Apply to the variable "T" with an initial condition
  temperature_name = T
  initial_temperature = 310 # [K]

  # Fix the outer boundary temperature to the fluid boundary temperature
  fixed_temperature_boundaries = water_solid_interface
  boundary_temperatures = T_fluid # [K]

  # Insulate the inner boundary (zero heat flux)
  insulated_boundaries = inner

  # Name of the material properties
  thermal_conductivity = k
  specific_heat = cp
  density = rho

  # Numerical parameters
  use_automatic_differentiation = false
[]

# Apply a constant heat source to the fuel
[Kernels/heat_source]
  type = BodyForce
  variable = T
  value = 1e8 # [W/m^2 in 2D]
[]

[Materials]
  [fuel]
    type = GenericConstantMaterial
    prop_names = 'k cp rho'
    prop_values = '2 3100 10700' # [W/m*K], [W/K*kg], [kg/m3]
    block = fuel
  []
  [clad]
    type = GenericConstantMaterial
    prop_names = 'k cp rho'
    prop_values = '10 2800 5400' # [W/m*K], [W/K*kg], [kg/m3]
    block = clad
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]

[Postprocessors]
  [T_max]
    type = NodalExtremeValue
    variable = T
  []
[]
