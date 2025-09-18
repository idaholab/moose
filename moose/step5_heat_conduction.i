hole_diameter = 0.16e-2    # [m]
pellet_diameter = 0.52e-2. # [m]
clad_diameter = 0.6e-2     # [m]
pin_pitch = 0.695e-2       # [m]

[Mesh]
  [concentric_circle]
    type = ConcentricCircleMeshGenerator
    num_sectors = 12
    radii = '${fparse hole_diameter/2} ${fparse pellet_diameter/2} ${fparse clad_diameter/2}'
    has_outer_square = false
    preserve_volumes = false
    rings = '1 3 1'
    pitch = ${pin_pitch}
  []
  [delete_hole]
    type = BlockDeletionGenerator
    input = concentric_circle
    block = 1
    new_boundary = inner
  []
  [rename_blocks]
    type = RenameBlockGenerator
    input = delete_hole
    old_block = '2 3'
    new_block = 'fuel clad'
  []
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
