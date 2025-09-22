[Mesh/fuel_pin]
  type = FileMeshGenerator
  file = ../step1_input_and_meshing/fuel_pin_in.e
[]

[Physics/HeatConduction/FiniteElement/heat_conduction]
  # Solve heat conduction on fuel and cladding
  block = 'fuel clad'

  # Store temperature as the variable "T"
  temperature_name = T

  # Fix the outer boundary temperature to the
  # fluid boundary temperature, pulled in from
  # the aux variable "T_fluid"
  # (for now is the value 300 K)
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

# Define a field variable that stores the temperature of
# fluid on the outer boundary (defined in the clad because
# boundary restriction not currently possible)
[AuxVariables/T_fluid]
  initial_condition = 300 # [K]
  boundary = clad
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

[Postprocessors]
  [T_max]
    type = NodalExtremeValue
    variable = T
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
