hole_diameter = 0.16e-2    # [m]
pellet_diameter = 0.52e-2  # [m]
clad_diameter = 0.6e-2     # [m]
pin_pitch = 0.695e-2       # [m]

# Fraction of the fluid thickness to add a boundary layer to
boundary_layer_frac = 0.1

boundary_layer_radius = ${fparse ((pin_pitch - clad_diameter) * boundary_layer_frac + clad_diameter) / 2}

[Mesh]
  [concentric_circle]
    type = ConcentricCircleMeshGenerator
    num_sectors = 12
    radii = '${fparse hole_diameter/2} ${fparse pellet_diameter/2} ${fparse clad_diameter/2} ${boundary_layer_radius}'
    has_outer_square = true
    preserve_volumes = false
    rings = '1 3 1 1 3'
    pitch = ${pin_pitch}
  []
  [delete_pin]
    type = BlockDeletionGenerator
    input = concentric_circle
    block = '1 2 3'
    new_boundary = inner
  []
  [rename_merge_blocks]
    type = RenameBlockGenerator
    input = delete_pin
    old_block = '4 5'
    new_block = 'water water'
  []
  [rename_outer]
    type = RenameBoundaryGenerator
    input = rename_merge_blocks
    old_boundary = 'top bottom right left'
    new_boundary = 'outer outer outer outer'
  []
[]

[Physics/HeatConduction/FiniteElement/heat_conduction]
  block = 'water'

  boundary_temperatures = 300 # [K]
  fixed_temperature_boundaries = outer

  insulated_boundaries = inner

  thermal_conductivity = k
[]

[Variables]
  [T] # [K]
  []
[]

[Materials]
  [k_water]
    type = ADGenericConstantMaterial
    prop_names = k
    prop_values = 0.6 # [W/m*K]
    block = water
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
