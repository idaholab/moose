[Mesh]
  [assm_up]
    type = TriPinHexAssemblyGenerator
    ring_radii = '7 8;5 6; '
    ring_intervals = '2 1;1 1; '
    ring_block_ids = '200 400 600;700 800; '
    background_block_ids = '30 40'
    num_sectors_per_side = 6
    background_intervals = 2
    hexagon_size = ${fparse 40.0/sqrt(3.0)}
    ring_offset = 0.6
    external_boundary_id = 200
    external_boundary_name = 'surface'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[AuxVariables]
  [pin_id]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [set_pin_id]
    type = ExtraElementIDAux
    variable = pin_id
    extra_id_name = pin_id
  []
[]

[Outputs]
  exodus = true
  execute_on = timestep_end
[]
