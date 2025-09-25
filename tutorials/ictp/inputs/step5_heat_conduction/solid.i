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

[AuxVariables]
  # Define an aux field variable that stores the temperature of
  # fluid on the outer boundary
  [T_fluid]
    initial_condition = 300 # [K]
    block = clad
  []
  # Define an aux field variable that stores the heat flux
  # on the outer boundary (the fluid interface)
  # To store the heat flux computation
  [heat_flux]
    # We compute the heat flux on the boundary cell
    # centers rather than nodes because linear-Lagrange
    # T does not have a well-defined gradient at nodes
    order = CONSTANT
    family = MONOMIAL
    block = clad
  []
[]

# Define an aux kernel that fills in the heat
# flux at the outer interface
[AuxKernels/heat_flux]
  type = DiffusionFluxAux
  variable = heat_flux
  diffusivity = k
  diffusion_variable = T
  boundary = 'water_solid_interface'
  component = normal
[]

[Postprocessors]
  # Compute the maximum temperature
  [T_max]
    type = NodalExtremeValue
    variable = T
  []

  # Compute the integral of the outgoing heat flux
  [heat_flux_integral]
    type = SideIntegralVariablePostprocessor
    variable = heat_flux
    boundary = 'water_solid_interface'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
  csv = true
[]
