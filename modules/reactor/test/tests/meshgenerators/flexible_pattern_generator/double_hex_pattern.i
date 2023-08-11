[Mesh]
  [accg]
    type = AdvancedConcentricCircleGenerator
    num_sectors = 9
    ring_radii = '1 2'
    ring_intervals = '2 2'
    ring_block_ids = '10 15 20'
    ring_block_names = 'inner_tri inner outer'
    external_boundary_id = 100
    external_boundary_name = 'ext'
    create_outward_interface_boundaries = false
  []
  [reduced_accg]
    type = TransformGenerator
    input = 'accg'
    transform = SCALE
    vector_value = '0.2 0.2 0.2'
  []
  [fpg]
    type = FlexiblePatternGenerator
    inputs = 'accg reduced_accg'
    boundary_type = HEXAGON
    boundary_sectors = 10
    boundary_size = ${fparse 12.0*sqrt(3.0)}
    hex_patterns = '0 0;
                   0 0 0;
                    0 0|
                    1 1;
                   1 1 1;
                    1 1'
    hex_pitches = '6 6'
    hex_origins = '0.0 0.0 0.0
                   2.0 2.0 0.0'
    desired_area = 1.0
  []
[]

[Problem]
  solve = false
[]

[Postprocessors]
  [background]
    type = VolumePostprocessor
    block = 0
  []
  [circle1]
    type = VolumePostprocessor
    block = '10 15'
  []
  [circle2]
    type = VolumePostprocessor
    block = '20'
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
    file_base = 'double_hex_pattern'
  []
[]
