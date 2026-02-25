box_width = 19.0
box_length = 15.0
box_height = 14.0

cylinder_wall_thickness = 1.0
cylinder_outer_radius = 23.0
cylinder_height = 35.0
cylinder_inner_radius = '${fparse cylinder_outer_radius - cylinder_wall_thickness}'

top_right_theta = '${fparse atan(box_length/box_width)}'

approximate_mesh_size = 0.5 # meters
nx_box = '${fparse ceil(box_width / approximate_mesh_size)}'
ny_box = '${fparse ceil(box_length / approximate_mesh_size)}'
nz_box = '${fparse ceil(box_height / approximate_mesh_size)}'
nr_air = '${fparse ceil((cylinder_inner_radius - 0.5 * sqrt(box_length*box_length + box_width*box_width)) / approximate_mesh_size)}'
nr_wall = '${fparse ceil(cylinder_wall_thickness / approximate_mesh_size)}'
nz_cylinder = '${fparse ceil(cylinder_height / approximate_mesh_size)}'

[Mesh]
  [box]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = '${fparse -box_width / 2}'
    xmax = '${fparse box_width / 2}'
    ymin = '${fparse -box_length / 2}'
    ymax = '${fparse box_length / 2}'
    nx = ${nx_box}
    ny = ${ny_box}
    subdomain_ids = '0'
  []

  [top_right_box_curve]
    type = ParsedCurveGenerator
    section_bounding_t_values = '0 1 2'
    nums_segments = '${ny_box} ${nx_box}'
    x_formula = 'if(t<=1, ${box_width} / 2, ${box_width} / 2 - (t - 1) * ${box_width})'
    y_formula = 'if(t<=1, t * ${box_length} - ${box_length} / 2, ${box_length} / 2)'
  []
  [top_right_air_curve]
    type = ParsedCurveGenerator
    nums_segments = '${fparse nx_box + ny_box}'
    section_bounding_t_values = '${top_right_theta} ${fparse pi + top_right_theta}'
    x_formula = 'cos(${fparse pi} - t) * ${cylinder_inner_radius}'
    y_formula = 'sin(${fparse pi} - t) * ${cylinder_inner_radius}'
  []
  [top_right_wall_curve]
    type = ParsedCurveGenerator
    nums_segments = '${fparse nx_box + ny_box}'
    section_bounding_t_values = '-${top_right_theta} ${fparse pi - top_right_theta}'
    x_formula = 'cos(t) * ${cylinder_outer_radius}'
    y_formula = 'sin(t) * ${cylinder_outer_radius}'
  []

  [top_right_air]
    type = FillBetweenCurvesGenerator
    input_mesh_1 = top_right_box_curve
    input_mesh_2 = top_right_air_curve
    num_layers = ${nr_air}
    use_quad_elements = true
    block_id = 1
  []
  [top_right_wall]
    type = FillBetweenCurvesGenerator
    input_mesh_1 = top_right_air_curve
    input_mesh_2 = top_right_wall_curve
    num_layers = ${nr_wall}
    use_quad_elements = true
    block_id = 2
  []

  [bottom_left_air]
    type = TransformGenerator
    input = top_right_air
    transform = ROTATE
    vector_value = '180 0 0'
  []
  [bottom_left_wall]
    type = TransformGenerator
    input = top_right_wall
    transform = ROTATE
    vector_value = '180 0 0'
  []

  [combine_2d]
    type = CombinerGenerator
    inputs = 'box top_right_air top_right_wall bottom_left_air bottom_left_wall'
  []
  [stitch_2d]
    type = MeshRepairGenerator
    input = combine_2d
    fix_node_overlap = true
  []

  [cylinder]
    type = AdvancedExtruderGenerator
    input = stitch_2d
    direction = '0 0 1'
    heights = '${box_height} ${fparse cylinder_height - box_height}'
    num_layers = '${nz_box} ${fparse nz_cylinder - nz_box}'
    subdomain_swaps = '0 0; 0 1'
    bottom_boundary = 'ground'
    top_boundary = 'ceiling'
  []

  [outer_boundary]
    type = ParsedGenerateSideset
    input = cylinder
    combinatorial_geometry = '1'
    new_sideset_name = 'outer_wall'
    include_only_external_sides = true
    excluded_boundaries = 'ground ceiling'
    included_subdomains = '2'
  []
  [air_wall_boundary]
    type = SideSetsBetweenSubdomainsGenerator
    input = outer_boundary
    new_boundary = air_wall_boundary
    primary_block = '1'
    paired_block = '2'
  []
  [air_box_boundary]
    type = SideSetsBetweenSubdomainsGenerator
    input = air_wall_boundary
    new_boundary = air_box_boundary
    primary_block = '1'
    paired_block = '0'
  []

  [remove_boundaries]
    type = BoundaryDeletionGenerator
    input = air_box_boundary
    boundary_names = 'ground ceiling outer_wall air_wall_boundary air_box_boundary'
    operation = keep
  []
[]
