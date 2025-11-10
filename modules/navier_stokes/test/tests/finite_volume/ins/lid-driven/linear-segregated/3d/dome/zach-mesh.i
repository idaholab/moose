box_width = 19.0
box_length = 15.0
box_height = 14.0

cylinder_wall_thickness = 1.0
cylinder_outer_radius = 23.0
cylinder_height = 35.0
cylinder_inner_radius = '${fparse cylinder_outer_radius - cylinder_wall_thickness}'

slab_outer_radius = 25.0
slab_thickness = 3.0

top_right_theta = '${fparse atan(box_length/box_width)}'

ppin_height = ${fparse slab_thickness + box_height}

# iolet_height = 7.9248
# ilet_width = 0.5472
# ilet_length = 0.4064
# olet_width = 0.9144
# olet_length = 0.1524
# ilet1_theta = '${fparse 4*pi/3}'
# ilet2_theta = '${fparse 3*pi/2}'
# ilet3_theta = '${fparse 5*pi/3}'
# olet1_theta = '${fparse pi/2}'
# olet2_theta = '${fparse 2*pi/3}'

approximate_mesh_size = 0.5 # meters
nx_box = '${fparse ceil(box_width / approximate_mesh_size)}'
ny_box = '${fparse ceil(box_length / approximate_mesh_size)}'
nz_box = '${fparse ceil(box_height / approximate_mesh_size)}'
nr_air = '${fparse ceil((cylinder_inner_radius - 0.5 * sqrt(box_length*box_length + box_width*box_width)) / approximate_mesh_size)}'
nr_wall = '${fparse ceil(cylinder_wall_thickness / approximate_mesh_size)}'
nr_slab = '${fparse ceil((slab_outer_radius - cylinder_outer_radius) / approximate_mesh_size)}'
nz_cylinder = '${fparse ceil(cylinder_height / approximate_mesh_size)}'
nz_slab = '${fparse ceil(slab_thickness / approximate_mesh_size)}'

[Mesh]
  [box_with_boundaries]
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
  [box]
    type = BoundaryDeletionGenerator
    input = box_with_boundaries
    boundary_names = 'top right bottom left'
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
  [top_right_slab_curve]
    type = ParsedCurveGenerator
    nums_segments = '${fparse nx_box + ny_box}'
    section_bounding_t_values = '${top_right_theta} ${fparse pi + top_right_theta}'
    x_formula = 'cos(${fparse pi} - t) * ${slab_outer_radius}'
    y_formula = 'sin(${fparse pi} - t) * ${slab_outer_radius}'
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
  [top_right_slab]
    type = FillBetweenCurvesGenerator
    input_mesh_1 = top_right_wall_curve
    input_mesh_2 = top_right_slab_curve
    num_layers = ${nr_slab}
    use_quad_elements = true
    block_id = 3
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
  [bottom_left_slab]
    type = TransformGenerator
    input = top_right_slab
    transform = ROTATE
    vector_value = '180 0 0'
  []

  [combine_2d]
    type = CombinerGenerator
    inputs = 'box top_right_air top_right_wall top_right_slab bottom_left_air bottom_left_wall bottom_left_slab'
  []
  [stitch_2d]
    type = MeshRepairGenerator
    input = combine_2d
    fix_node_overlap = true
  []

  [cylinder_with_slab_extrusion]
    type = AdvancedExtruderGenerator
    input = stitch_2d
    direction = '0 0 1'
    heights = '${slab_thickness} ${box_height} ${fparse cylinder_height - box_height}'
    num_layers = '${nz_slab} ${nz_box} ${fparse nz_cylinder - nz_box}'
    subdomain_swaps = '0 2  1 2  2 2  3 2; 3 99; 0 1  3 99'
    bottom_boundary = 'ground'
    downward_boundary_ids = "700 700 700 700; 700 500 700 700; 700 700 700 700"
    downward_boundary_source_blocks = '0 1 2 3; 0 1 2 3; 0 1 2 3'
  []
  [remove_dummy_boundary]
    type = BoundaryDeletionGenerator
    input = cylinder_with_slab_extrusion
    boundary_names = '700'
  []
  [rename_room_floor]
    type = RenameBoundaryGenerator
    input = remove_dummy_boundary
    old_boundary = '500'
    new_boundary = 'room_floor_placeholder'
  []

  [cylinder]
    type = BlockDeletionGenerator
    input = rename_room_floor
    block = 99
    new_boundary = outer_wall
  []

  [disk_for_dome]
    type = BlockDeletionGenerator
    input = stitch_2d
    block = 3
  []
  [cylinder_for_dome]
    type = AdvancedExtruderGenerator
    input = disk_for_dome
    direction = '0 0 1'
    heights = '${cylinder_inner_radius} ${cylinder_wall_thickness}'
    num_layers = '${nr_air} ${nr_wall}'
    subdomain_swaps = '0 1; 0 2  1 2'
  []
  [dome]
    type = ParsedNodeTransformGenerator
    input = cylinder_for_dome
    x_function = 'r:=sqrt(x*x + y*y + z*z);
                  rxy:=sqrt(x*x + y*y);
                  theta:=atan2(y, x);
                  phi:=acos(z/r);
                  f:=if(rxy<z, z/r, rxy/r);
                  if(z>0, f*x, x)'
    y_function = 'r:=sqrt(x*x + y*y + z*z);
                  rxy:=sqrt(x*x + y*y);
                  theta:=atan2(y, x);
                  phi:=acos(z/r);
                  f:=if(rxy<z, z/r, rxy/r);
                  if(z>0, f*y, y)'
    z_function = 'r:=sqrt(x*x + y*y + z*z);
                  rxy:=sqrt(x*x + y*y);
                  theta:=atan2(y, x);
                  phi:=acos(z/r);
                  f:=if(rxy<z, z/r, rxy/r);
                  if(z>0, f*z, 0)'

    constant_names = 'R'
    constant_expressions = '${cylinder_outer_radius}'
  []

  [combine_3d]
    type = CombinerGenerator
    inputs = 'cylinder dome'
    positions = '0 0 -${slab_thickness}  0 0 ${cylinder_height}'
    avoid_merging_boundaries = true
  []
  [stitch_3d]
    type = MeshRepairGenerator
    input = combine_3d
    fix_node_overlap = true
  []

  [outer_boundary]
    type = ParsedGenerateSideset
    input = stitch_3d
    combinatorial_geometry = '1'
    new_sideset_name = 'outer_wall'
    include_only_external_sides = true
    excluded_boundaries = 'ground'
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

  # [inlet_boundary]
  #   type = ParsedGenerateSideset
  #   input = air_box_boundary
  #   combinatorial_geometry = 'tpi:=atan2(y, x);t:=if(tpi < 0, ${fparse pi} - tpi, tpi);
  #                             z >= (${iolet_height} - lh) & z < (${iolet_height} + lh) &
  #                             ((t >= (${ilet1_theta} - th) & t < (${ilet1_theta} + th)) |
  #                              (t >= (${ilet2_theta} - th) & t < (${ilet2_theta} + th)) |
  #                              (t >= (${ilet3_theta} - th) & t < (${ilet3_theta} + th)))'
  #   constant_names = 'lh th'
  #   constant_expressions = '${fparse max(ilet_length, approximate_mesh_size) / 2}
  #                           ${fparse max(ilet_width / cylinder_inner_radius, pi / (nx_box + ny_box)) / 2}'
  #   new_sideset_name = inlet
  #   included_boundaries = air_wall_boundary
  # []
  # [outlet_boundary]
  #   type = ParsedGenerateSideset
  #   input = inlet_boundary
  #   combinatorial_geometry = 'tpi:=atan2(y, x);t:=if(tpi < 0, ${fparse pi} - tpi, tpi);
  #                             z >= (${iolet_height} - lh) & z < (${iolet_height} + lh) &
  #                             ((t >= (${olet1_theta} - th) & t < (${olet1_theta} + th)) |
  #                              (t >= (${olet2_theta} - th) & t < (${olet2_theta} + th)))'
  #   constant_names = 'lh th'
  #   constant_expressions = '${fparse max(olet_length, approximate_mesh_size) / 2}
  #                           ${fparse max(olet_width / cylinder_inner_radius, pi / (nx_box + ny_box)) / 2}'
  #   new_sideset_name = outlet
  #   included_boundaries = air_wall_boundary
  # []

  [remove_boundaries]
    type = BoundaryDeletionGenerator
    # input = outlet_boundary
    # boundary_names = 'ground outer_wall air_wall_boundary air_box_boundary inlet outlet'
    input = air_box_boundary
    boundary_names = 'room_floor_placeholder ground outer_wall air_wall_boundary air_box_boundary'
    operation = keep
  []

  # Only need block 1 for fluid natural convection simulation
  [delete]
    type = BlockDeletionGenerator
    input = remove_boundaries
    block = '0 2'
  []

  [remove_selection_of_air_wall_boundary]
    type = SideSetsFromNormalsGenerator
    normals = '0 0 -1'
    included_boundaries = "room_floor_placeholder"
    replace = true
    new_boundary = 'room_floor'
    input = delete
  []
[]
