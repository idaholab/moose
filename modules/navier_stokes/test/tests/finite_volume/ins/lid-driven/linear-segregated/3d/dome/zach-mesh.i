box_width = 9.753124047546478
box_length = 5.7401198821497506
box_height = 4.241593010261099

cylinder_outer_radius = '${fparse 24.41 / 2}'
cylinder_inner_radius = '${fparse 23.75 / 2}'
cylinder_wall_thickness = '${fparse cylinder_outer_radius - cylinder_inner_radius}'
cylinder_height = 14.40

top_right_theta = '${fparse atan(box_length/box_width)}'

olet1_theta = 2.1917869591634602 # 125.58014235187879 deg
olet1_height = 7.537336177994514
olet1_width = 0.45717768972874123
olet1_length = 0.40638016864776993

olet2_theta = 1.5345842874203033 # 87.92520297627425 deg
olet2_height = 7.537336177994514
olet2_width = 0.45717768972874123
olet2_length = 0.40638016864776993

ilet1_theta = 5.136622754957465 # 294.3068048099244 deg
ilet1_height = 5.555013715330691
ilet1_width = 0.7619628162145687
ilet1_length = 0.15239256324291373

ilet2_theta = 4.393598248211603 # 251.73463649859679 deg
ilet2_height = 5.555013715330691
ilet2_width = 0.7619628162145687
ilet2_length = 0.15239256324291373

ilet3_theta = 4.024280858846884 # 230.57430878720865 deg
ilet3_height = 5.555013715330691
ilet3_width = 0.7619628162145687
ilet3_length = 0.15239256324291373

dfan_elevation = 9.143553794574824
dfan_diameter = 0.5587727318906837
dfan_thickness = 0.30478512648582745
dfan1_theta = 0.13946341426455391 # 7.990665033843541 deg
dfan2_theta = 4.3284370960839285 # 248.0011774934711 deg
dfan3_theta = 2.2338205316843047 # 127.98848865518025 deg

ahu_vent_width = 0.7111652951335975
ahu_vent_height = 0.5587727318906837
ahu_width = '${fparse 5.25 + 0.5 + ahu_vent_height}'
ahu_height = '${fparse 6.45 + 0.5 + ahu_vent_height}'
ahu_depth = 1.6
ahu_theta = 5.759586531581287
ahu_intake_bottom = 3
ahu_intake_height = '${fparse ahu_vent_width * 3 + 0.2 * 2}'
ahu_exhaust_left = 0
ahu_exhaust_width = '${fparse ahu_vent_width * 3 + 0.6 * 2}'

boundary_layer_thickness = 0.5 # meters
bl_box_width = '${fparse box_width + boundary_layer_thickness * 2}'
bl_box_length = '${fparse box_length + boundary_layer_thickness * 2}'
bl_wall_radius = '${fparse cylinder_inner_radius - boundary_layer_thickness}'

approximate_mesh_size = 0.25 # meters
nx_box = '${fparse ceil(box_width / approximate_mesh_size)}'
ny_box = '${fparse ceil(box_length / approximate_mesh_size)}'
nz_box = '${fparse ceil(box_height / approximate_mesh_size)}'
nr_air = '${fparse ceil((cylinder_inner_radius - 0.5 * sqrt(box_length + box_width*box_width) - ahu_depth) / approximate_mesh_size)}'
nr_wall = '${fparse ceil(cylinder_wall_thickness / approximate_mesh_size)}'
nr_ahu = '${fparse ceil((ahu_depth - boundary_layer_thickness) / approximate_mesh_size)}'
nz_cylinder = '${fparse ceil(cylinder_height / approximate_mesh_size)}'
nz_dfan = '2'

boundary_layer_cell_size = 0.25 # meters
n_bl = '${fparse ceil(boundary_layer_thickness / boundary_layer_cell_size)}'

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
    nums_segments = '${nx_box} ${ny_box}'
    x_formula = 'if(t<=1, t * ${box_width} - ${box_width} / 2, ${box_width} / 2)'
    y_formula = 'if(t<=1, ${box_length} / 2, ${box_length} / 2 - (t - 1) * ${box_length})'
  []
  [top_right_box_bl_curve]
    type = ParsedCurveGenerator
    section_bounding_t_values = '0 1 2'
    nums_segments = '${ny_box} ${nx_box}'
    x_formula = 'if(t<=1, ${bl_box_width} / 2, ${bl_box_width} / 2 - (t - 1) * ${bl_box_width})'
    y_formula = 'if(t<=1, t * ${bl_box_length} - ${bl_box_length} / 2, ${bl_box_length} / 2)'
  []
  [top_right_ahu_curve]
    type = ParsedCurveGenerator
    nums_segments = '${fparse nx_box + ny_box}'
    section_bounding_t_values = '${top_right_theta} ${fparse pi + top_right_theta}'
    x_formula = 'cos(${fparse pi} - t) * ${fparse cylinder_inner_radius - ahu_depth}'
    y_formula = 'sin(${fparse pi} - t) * ${fparse cylinder_inner_radius - ahu_depth}'
  []
  [top_right_wall_bl_curve]
    type = ParsedCurveGenerator
    nums_segments = '${fparse nx_box + ny_box}'
    # section_bounding_t_values = '${top_right_theta} ${fparse pi + top_right_theta}'
    # x_formula = 'cos(${fparse pi} - t) * ${bl_wall_radius}'
    # y_formula = 'sin(${fparse pi} - t) * ${bl_wall_radius}'
    section_bounding_t_values = '-${top_right_theta} ${fparse pi - top_right_theta}'
    x_formula = 'cos(t) * ${bl_wall_radius}'
    y_formula = 'sin(t) * ${bl_wall_radius}'
  []
  [top_right_air_curve]
    type = ParsedCurveGenerator
    nums_segments = '${fparse nx_box + ny_box}'
    # section_bounding_t_values = '-${top_right_theta} ${fparse pi - top_right_theta}'
    # x_formula = 'cos(t) * ${cylinder_inner_radius}'
    # y_formula = 'sin(t) * ${cylinder_inner_radius}'
    section_bounding_t_values = '${top_right_theta} ${fparse pi + top_right_theta}'
    x_formula = 'cos(${fparse pi} - t) * ${cylinder_inner_radius}'
    y_formula = 'sin(${fparse pi} - t) * ${cylinder_inner_radius}'
  []
  [top_right_wall_curve]
    type = ParsedCurveGenerator
    nums_segments = '${fparse nx_box + ny_box}'
    # section_bounding_t_values = '${top_right_theta} ${fparse pi + top_right_theta}'
    # x_formula = 'cos(${fparse pi} - t) * ${cylinder_outer_radius}'
    # y_formula = 'sin(${fparse pi} - t) * ${cylinder_outer_radius}'
    section_bounding_t_values = '-${top_right_theta} ${fparse pi - top_right_theta}'
    x_formula = 'cos(t) * ${cylinder_outer_radius}'
    y_formula = 'sin(t) * ${cylinder_outer_radius}'
  []

  [top_right_box_bl]
    type = FillBetweenCurvesGenerator
    input_mesh_1 = top_right_box_curve
    input_mesh_2 = top_right_box_bl_curve
    num_layers = ${n_bl}
    use_quad_elements = true
    block_id = 1
  []
  [top_right_air]
    type = FillBetweenCurvesGenerator
    input_mesh_1 = top_right_box_bl_curve
    input_mesh_2 = top_right_ahu_curve
    num_layers = ${nr_air}
    bias_parameter = 1.0
    use_quad_elements = true
    block_id = 1
  []
  [top_right_ahu]
    type = FillBetweenCurvesGenerator
    input_mesh_1 = top_right_ahu_curve
    input_mesh_2 = top_right_wall_bl_curve
    num_layers = ${nr_ahu}
    bias_parameter = 1.0
    use_quad_elements = true
    block_id = 1
  []
  [top_right_wall_bl]
    type = FillBetweenCurvesGenerator
    input_mesh_1 = top_right_wall_bl_curve
    input_mesh_2 = top_right_air_curve
    num_layers = ${n_bl}
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

  [bottom_left_box_bl]
    type = TransformGenerator
    input = top_right_box_bl
    transform = ROTATE
    vector_value = '180 0 0'
  []
  [bottom_left_air]
    type = TransformGenerator
    input = top_right_air
    transform = ROTATE
    vector_value = '180 0 0'
  []
  [bottom_left_ahu]
    type = TransformGenerator
    input = top_right_ahu
    transform = ROTATE
    vector_value = '180 0 0'
  []
  [bottom_left_wall_bl]
    type = TransformGenerator
    input = top_right_wall_bl
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
    inputs = 'box
              top_right_box_bl top_right_air top_right_ahu top_right_wall_bl top_right_wall
              bottom_left_box_bl bottom_left_air bottom_left_ahu bottom_left_wall_bl bottom_left_wall'
  []
  [stitch_2d]
    type = MeshRepairGenerator
    input = combine_2d
    fix_node_overlap = true
  []

  [destratification_1_block]
    type = ParsedSubdomainMeshGenerator
    input = stitch_2d
    block_id = 10
    combinatorial_geometry = 'tpi:=atan2(y, x);t:=if(tpi < 0, ${fparse 2 * pi} + tpi, tpi);r:=sqrt(x^2 + y^2);
                              t >= (${dfan1_theta} - th) & t < (${dfan1_theta} + th) &
                              r >= ${fparse cylinder_inner_radius - dfan_diameter} & r < ${cylinder_inner_radius}'
    constant_names = 'th'
    constant_expressions = '${fparse max(dfan_diameter / cylinder_inner_radius, pi / (nx_box + ny_box)) / 2}'
  []
  [destratification_2_block]
    type = ParsedSubdomainMeshGenerator
    input = destratification_1_block
    block_id = 11
    combinatorial_geometry = 'tpi:=atan2(y, x);t:=if(tpi < 0, ${fparse 2 * pi} + tpi, tpi);r:=sqrt(x^2 + y^2);
                              t >= (${dfan2_theta} - th) & t < (${dfan2_theta} + th) &
                              r >= ${fparse cylinder_inner_radius - dfan_diameter} & r < ${cylinder_inner_radius}'
    constant_names = 'th'
    constant_expressions = '${fparse max(dfan_diameter / cylinder_inner_radius, pi / (nx_box + ny_box)) / 2}'
  []
  [destratification_3_block]
    type = ParsedSubdomainMeshGenerator
    input = destratification_2_block
    block_id = 12
    combinatorial_geometry = 'tpi:=atan2(y, x);t:=if(tpi < 0, ${fparse 2 * pi} + tpi, tpi);r:=sqrt(x^2 + y^2);
                              t >= (${dfan3_theta} - th) & t < (${dfan3_theta} + th) &
                              r >= ${fparse cylinder_inner_radius - dfan_diameter} & r < ${cylinder_inner_radius}'
    constant_names = 'th'
    constant_expressions = '${fparse max(dfan_diameter / cylinder_inner_radius, pi / (nx_box + ny_box)) / 2}'
  []

  [cylinder]
    type = AdvancedExtruderGenerator
    input = destratification_3_block
    direction = '0 0 1'
    heights = '${box_height}
               ${boundary_layer_thickness}
               ${fparse dfan_elevation - dfan_thickness - boundary_layer_thickness - box_height}
               ${dfan_thickness}
               ${fparse cylinder_height - dfan_elevation}'
    num_layers = '${nz_box} ${n_bl} ${fparse ceil((nz_cylinder - nz_box) / 2)} ${nz_dfan} ${fparse ceil((nz_cylinder - nz_box) / 2)}'
    subdomain_swaps = '10 1  11 1  12 1; 0 1  10 1  11 1  12 1; 0 1  10 1  11 1  12 1; 0 1; 0 1  10 1  11 1  12 1'
  []

  [cylinder_for_dome]
    type = AdvancedExtruderGenerator
    input = stitch_2d
    direction = '0 0 1'
    heights = '${bl_wall_radius} ${boundary_layer_thickness} ${cylinder_wall_thickness}'
    num_layers = '${nr_air} ${n_bl} ${nr_wall}'
    subdomain_swaps = '0 1; 0 1; 0 2  1 2'
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
    positions = '0 0 0  0 0 ${cylinder_height}'
    avoid_merging_boundaries = true
  []
  [stitch_3d]
    type = MeshRepairGenerator
    input = combine_3d
    fix_node_overlap = true
  []

  [ahu_block]
    type = ParsedSubdomainMeshGenerator
    input = stitch_3d
    block_id = 20
    combinatorial_geometry = 'r:=sqrt(x^2 + y^2);tpi:=atan2(y, x);t:=if(tpi < 0, ${fparse  2 * pi} + tpi, tpi);
                              r >= ${fparse cylinder_inner_radius - ahu_depth} & r < ${cylinder_inner_radius} &
                              t >= (${ahu_theta} - th) & t < (${ahu_theta} + th) &
                              z < ${ahu_height}'
    constant_names = 'th'
    constant_expressions = '${fparse ahu_width / 2 / cylinder_inner_radius}'
  []

  [air_floor_boundary]
    type = SideSetsFromNormalsGenerator
    input = ahu_block
    new_boundary = 'air_floor_boundary'
    normals = '0 0 -1'
    included_subdomains = '1'
  []
  [wall_floor_boundary]
    type = SideSetsFromNormalsGenerator
    input = air_floor_boundary
    new_boundary = 'wall_floor_boundary'
    normals = '0 0 -1'
    included_subdomains = '2'
  []
  [outer_boundary]
    type = ParsedGenerateSideset
    input = wall_floor_boundary
    combinatorial_geometry = '1'
    new_sideset_name = 'wall_outer_boundary'
    include_only_external_sides = true
    excluded_boundaries = 'wall_floor_boundary'
    included_subdomains = '2'
  []
  [air_wall_boundary]
    type = SideSetsBetweenSubdomainsGenerator
    input = outer_boundary
    new_boundary = air_wall_boundary
    primary_block = '1 10 11 12'
    paired_block = '2 2 2 2'
  []
  [wall_air_boundary]
    type = SideSetsBetweenSubdomainsGenerator
    input = air_wall_boundary
    new_boundary = wall_air_boundary
    primary_block = '2 2 2 2'
    paired_block = '1 10 11 12'
  []
  [air_box_boundary]
    type = BlockDeletionGenerator
    input = wall_air_boundary
    block = '0'
    new_boundary = 'air_box_boundary'
  []

  [outlet_1_boundary]
    type = ParsedGenerateSideset
    input = air_box_boundary
    combinatorial_geometry = 'tpi:=atan2(y, x);t:=if(tpi < 0, ${fparse 2 * pi} + tpi, tpi);
                              z >= (${olet1_height} - lh) & z < (${olet1_height} + lh) &
                              t >= (${olet1_theta} - th) & t < (${olet1_theta} + th)'
    constant_names = 'lh th'
    constant_expressions = '${fparse max(olet1_length, approximate_mesh_size) / 2}
                            ${fparse max(olet1_width / cylinder_inner_radius, pi / (nx_box + ny_box)) / 2}'
    new_sideset_name = 'outlet_1'
    included_boundaries = air_wall_boundary
    replace = True
  []
  [outlet_2_boundary]
    type = ParsedGenerateSideset
    input = outlet_1_boundary
    combinatorial_geometry = 'tpi:=atan2(y, x);t:=if(tpi < 0, ${fparse 2 * pi} + tpi, tpi);
                              z >= (${olet2_height} - lh) & z < (${olet2_height} + lh) &
                              t >= (${olet2_theta} - th) & t < (${olet2_theta} + th)'
    constant_names = 'lh th'
    constant_expressions = '${fparse max(olet2_length, approximate_mesh_size) / 2}
                            ${fparse max(olet2_width / cylinder_inner_radius, pi / (nx_box + ny_box)) / 2}'
    new_sideset_name = 'outlet_2'
    included_boundaries = air_wall_boundary
    replace = True
  []
  [inlet_1_boundary]
    type = ParsedGenerateSideset
    input = outlet_2_boundary
    combinatorial_geometry = 'tpi:=atan2(y, x);t:=if(tpi < 0, ${fparse 2 * pi} + tpi, tpi);
                              z >= (${ilet1_height} - lh) & z < (${ilet1_height} + lh) &
                              t >= (${ilet1_theta} - th) & t < (${ilet1_theta} + th)'
    constant_names = 'lh th'
    constant_expressions = '${fparse max(ilet1_length, approximate_mesh_size) / 2}
                            ${fparse max(ilet1_width / cylinder_inner_radius, pi / (nx_box + ny_box)) / 2}'
    new_sideset_name = 'inlet_1'
    included_boundaries = air_wall_boundary
    replace = True
  []
  [inlet_2_boundary]
    type = ParsedGenerateSideset
    input = inlet_1_boundary
    combinatorial_geometry = 'tpi:=atan2(y, x);t:=if(tpi < 0, ${fparse 2 * pi} + tpi, tpi);
                              z >= (${ilet2_height} - lh) & z < (${ilet2_height} + lh) &
                              t >= (${ilet2_theta} - th) & t < (${ilet2_theta} + th)'
    constant_names = 'lh th'
    constant_expressions = '${fparse max(ilet2_length, approximate_mesh_size) / 2}
                            ${fparse max(ilet2_width / cylinder_inner_radius, pi / (nx_box + ny_box)) / 2}'
    new_sideset_name = 'inlet_2'
    included_boundaries = air_wall_boundary
    replace = True
  []
  [inlet_3_boundary]
    type = ParsedGenerateSideset
    input = inlet_2_boundary
    combinatorial_geometry = 'tpi:=atan2(y, x);t:=if(tpi < 0, ${fparse 2 * pi} + tpi, tpi);
                              z >= (${ilet3_height} - lh) & z < (${ilet3_height} + lh) &
                              t >= (${ilet3_theta} - th) & t < (${ilet3_theta} + th)'
    constant_names = 'lh th'
    constant_expressions = '${fparse max(ilet3_length, approximate_mesh_size) / 2}
                            ${fparse max(ilet3_width / cylinder_inner_radius, pi / (nx_box + ny_box)) / 2}'
    new_sideset_name = 'inlet_3'
    included_boundaries = air_wall_boundary
    replace = True
  []

  [air_ahu_boundary]
    type = SideSetsBetweenSubdomainsGenerator
    input = inlet_3_boundary
    new_boundary = air_ahu_boundary
    primary_block = '1'
    paired_block = '20'
  []
  [ahu_intake_boundary]
    type = ParsedGenerateSideset
    input = air_ahu_boundary
    new_sideset_name = ahu_intake
    combinatorial_geometry = 'r:=sqrt(x^2 + y^2);tpi:=atan2(y, x);t:=if(tpi < 0, ${fparse 2 * pi} + tpi, tpi);
                              r < ${fparse cylinder_inner_radius - ahu_depth + 0.1} &
                              t >= right & t < (right + ${fparse ahu_vent_height / cylinder_inner_radius}) &
                              z >= ${ahu_intake_bottom} & z < ${fparse ahu_intake_bottom + ahu_intake_height}'
    constant_names = 'right'
    constant_expressions = '${fparse ahu_theta - ahu_width / 2 / cylinder_inner_radius}'
    included_boundaries = air_ahu_boundary
    replace = true
  []
  [ahu_exhaust_boundary]
    type = ParsedGenerateSideset
    input = ahu_intake_boundary
    new_sideset_name = ahu_exhaust
    combinatorial_geometry = 'r:=sqrt(x^2 + y^2);tpi:=atan2(y, x);dt:=left - tpi;
                              r < ${fparse cylinder_inner_radius - ahu_depth + 0.1} &
                              dt >= 0 & dt < ${fparse ahu_exhaust_width / cylinder_inner_radius} &
                              z >= ${fparse ahu_height - ahu_vent_height} & z < ${ahu_height}'
    constant_names = 'left'
    constant_expressions = '${fparse ahu_theta + (ahu_width / 2 - ahu_exhaust_left) / cylinder_inner_radius - 2 * pi}'
    included_boundaries = air_ahu_boundary
    replace = true
  []
  [remove_boundaries]
    type = BoundaryDeletionGenerator
    input = ahu_exhaust_boundary
    boundary_names = 'air_floor_boundary air_wall_boundary air_box_boundary
                      wall_floor_boundary wall_air_boundary wall_outer_boundary
                      outlet_1 outlet_2 inlet_1 inlet_2 inlet_3
                      air_ahu_boundary ahu_intake ahu_exhaust'
    operation = keep
  []
[]

[Variables/u]
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [box]
    type = DirichletBC
    variable = u
    boundary = air_box_boundary
    value = 0
  []
  [outer]
    type = DirichletBC
    variable = u
    boundary = wall_outer_boundary
    value = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
