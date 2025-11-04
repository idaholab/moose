[Mesh]
  [accg_1]
    type = AdvancedConcentricCircleGenerator
    num_sectors = 9
    ring_radii = '2'
    ring_intervals = '1'
    ring_block_ids = '10'
    ring_block_names = 'accg_1'
    create_outward_interface_boundaries = false
  []
  [accg_2]
    type = AdvancedConcentricCircleGenerator
    num_sectors = 9
    ring_radii = '2'
    ring_intervals = '1'
    ring_block_ids = '20'
    ring_block_names = 'accg_2'
    create_outward_interface_boundaries = false
  []
  [accg_3]
    type = AdvancedConcentricCircleGenerator
    num_sectors = 9
    ring_radii = '2'
    ring_intervals = '1'
    ring_block_ids = '30'
    ring_block_names = 'accg_3'
    create_outward_interface_boundaries = false
  []
  [fpg]
    type = FlexiblePatternGenerator
    inputs = 'accg_1 accg_2 accg_3'
    boundary_type = HEXAGON
    boundary_size = ${fparse 12.0*sqrt(3.0)}
    boundary_sectors = 10

    extra_positions = '0.0 6.0 0.0
                       0.0 -6.0 0.0
                       0.0 0.0 0.0'
    extra_positions_mg_indices = '0 1 2'

    desired_area = 1.0
  []
[]

[Problem]
  solve = false
[]

[AuxVariables]
  [pin_id]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [pin_id]
    type = ExtraElementIDAux
    extra_id_name = pin_id
    variable = pin_id
  []
[]

[Postprocessors]
  [accg_1_pin_id_avg]
    type = ElementAverageValue
    variable = pin_id
    block = 10
  []
  [accg_2_pin_id_avg]
    type = ElementAverageValue
    variable = pin_id
    block = 20
  []
  [accg_3_pin_id_avg]
    type = ElementAverageValue
    variable = pin_id
    block = 30
  []
  [accg_1_pin_id_max]
    type = ElementExtremeValue
    variable = pin_id
    block = 10
  []
  [accg_2_pin_id_max]
    type = ElementExtremeValue
    variable = pin_id
    block = 20
  []
  [accg_3_pin_id_max]
    type = ElementExtremeValue
    variable = pin_id
    block = 30
  []
  [accg_1_pin_id_min]
    type = ElementExtremeValue
    variable = pin_id
    block = 10
    value_type = min
  []
  [accg_2_pin_id_min]
    type = ElementExtremeValue
    variable = pin_id
    block = 20
    value_type = min
  []
  [accg_3_pin_id_min]
    type = ElementExtremeValue
    variable = pin_id
    block = 30
    value_type = min
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  [csv]
    type = CSV
    execute_on = FINAL
  []
[]
