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
  [pcg]
    type = ParsedCurveGenerator
    x_formula = '10*cos(t)'
    y_formula = 'y1:=10*sin(t);
                   y2:=15*sin(t);
                   if(t<pi,y1,y2)'
    section_bounding_t_values = '0.0 ${fparse pi} ${fparse 2.0*pi}'
    nums_segments = '10 10'
    constant_names = 'pi'
    constant_expressions = '${fparse pi}'
    is_closed_loop = true
  []
  [fpg]
    type = FlexiblePatternGenerator
    inputs = 'accg'
    boundary_type = CUSTOM
    boundary_mesh = pcg
    hex_patterns = '0 0;
                   0 0 0;
                    0 0'
    hex_pitches = 6
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
    file_base = 'single_hex_pattern_custom'
  []
[]
