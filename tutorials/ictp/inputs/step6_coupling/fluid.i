[Mesh/fluid]
  type = FileMeshGenerator
  file = ../step1_input_and_meshing/fluid_in.e
[]

[Physics/HeatConduction/FiniteElement/heat_conduction]
  # Solve heat conduction on the water
  block = 'water'

  # Store temperature as the variable "T"
  temperature_name = T_fluid
  # ...with an initial condition of 300 K
  initial_temperature = 300

  # Prescribe the heat flux on the water solid
  # interface to that from the interface, pulled
  # in from the aux variable "flux_from_solid"
  # (for now is the value 500 W/M)
  heat_flux_boundaries = water_solid_interface
  boundary_heat_fluxes = flux_from_solid # [W/m2]

  # Evacuate heat at the outer boundary
  # with a prescribed temperature of 300K
  fixed_temperature_boundaries = outer
  boundary_temperatures = 300 # [K]

  # Name of the material properties
  thermal_conductivity = k
  specific_heat = cp
  density = rho
[]

# Define a field variable that stores the heat
# flux from the solid
[AuxVariables]
  [flux_from_solid]
    initial_condition = 2e4 # [W/m]
  []
[]

[Materials]
  [k_water]
    type = ADGenericConstantMaterial
    prop_names = 'k rho cp'
    prop_values = '0.6 1000 4100'
    block = water
  []
[]

[Executioner]
  type = Transient

  # Solver parameters
  automatic_scaling = true
  nl_abs_tol = 2e-12
  line_search = none
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
