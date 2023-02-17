# a Pronghorn mesh for 7 EBRT-II assemblies

# sqrt(3) / 2 is by how much flat to flat is smaller than corer to corner
f = ${fparse sqrt(3) / 2}

# units are cm - do not forget to convert to meter
outer_duct_out = 5.8166
outer_duct_in = 5.5854
inner_duct_out = 4.8437
inner_duct_in = 4.64
inter_wrapper_width = 0.3
height = 61.2
n_ax = 34

[Mesh]
  [XX09]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '4 4 4 4 4 4'
    background_intervals = 1
    background_block_ids = '12'
    polygon_size = ${fparse outer_duct_out / 2 + inter_wrapper_width / 2}
    duct_sizes = '${fparse inner_duct_in / f /2} ${fparse inner_duct_out / f /2} ${fparse outer_duct_in / f /2} ${fparse outer_duct_out / f / 2}'
    duct_intervals = '4    8    4    3'
    duct_block_ids = '1003 1004 1005 1006'
    interface_boundary_names = 'inner_wall_in inner_wall_out outer_wall_in outer_wall_out'
    interface_boundary_id_shift = 100
  []

  [hfd]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '4 4 4 4 4 4'
    background_intervals = 1
    background_block_ids = '13'
    polygon_size = ${fparse outer_duct_out / 2 + inter_wrapper_width / 2}
    duct_sizes = '${fparse 5.6134 / f /2} ${fparse outer_duct_out / f / 2}'
    duct_intervals = '8    3'
    duct_block_ids = '1003  1006'
    interface_boundary_names = 'wall_in wall_out'
  []

  [Partial_Driver]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '4 4 4 4 4 4'
    background_intervals = 1
    background_block_ids = '14'
    polygon_size = ${fparse outer_duct_out / 2 + inter_wrapper_width / 2}
    duct_sizes = '${fparse 5.6134 / f /2} ${fparse outer_duct_out / f / 2}'
    duct_intervals = '8    3'
    duct_block_ids = '1003  1006'
    interface_boundary_names = 'wall_in wall_out'
  []

  [Driver1]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '4 4 4 4 4 4'
    background_intervals = 1
    background_block_ids = '15'
    polygon_size = ${fparse outer_duct_out / 2 + inter_wrapper_width / 2}
    duct_sizes = '${fparse 5.6134 / f /2} ${fparse outer_duct_out / f / 2}'
    duct_intervals = '8    3'
    duct_block_ids = '1003  1006'
    interface_boundary_names = 'wall_in wall_out'
  []

  [Driver2]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '4 4 4 4 4 4'
    background_intervals = 1
    background_block_ids = '16'
    polygon_size = ${fparse outer_duct_out / 2 + inter_wrapper_width / 2}
    duct_sizes = '${fparse 5.6134 / f /2} ${fparse outer_duct_out / f / 2}'
    duct_intervals = '8    3'
    duct_block_ids = '1003  1006'
    interface_boundary_names = 'wall_in wall_out'
  []

  [K011]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '4 4 4 4 4 4'
    background_intervals = 1
    background_block_ids = '17'
    polygon_size = ${fparse outer_duct_out / 2 + inter_wrapper_width / 2}
    duct_sizes = '${fparse 5.6134 / f /2} ${fparse outer_duct_out / f / 2}'
    duct_intervals = '8    3'
    duct_block_ids = '1003  1006'
    interface_boundary_names = 'wall_in wall_out'
  []

  [X402]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '4 4 4 4 4 4'
    background_intervals = 1
    background_block_ids = '18'
    polygon_size = ${fparse outer_duct_out / 2 + inter_wrapper_width / 2}
    duct_sizes = '${fparse 5.6134 / f /2} ${fparse outer_duct_out / f / 2}'
    duct_intervals = '8    3'
    duct_block_ids = '1003  1006'
    interface_boundary_names = 'wall_in wall_out'
  []

  [pattern]
    type = PatternedHexMeshGenerator
    inputs = 'XX09 hfd Partial_Driver Driver1 Driver2 K011 X402'
    pattern =
              '5 4 ;
              6 0 3 ;
               1 2 '
    pattern_boundary = none
  []

  [extrude]
    type = AdvancedExtruderGenerator
    direction = '0 0 1'
    input = pattern
    heights = '${height}'
    num_layers = '${n_ax}'
  []

  [inlet_interwall]
    type = ParsedGenerateSideset
    input = extrude
    combinatorial_geometry = 'abs(z) < 1e-6'
    included_subdomains = 1004
    normal = '0 0 -1'
    new_sideset_name = inlet_interwall
  []

  [inlet_interwrapper]
    type = ParsedGenerateSideset
    input = inlet_interwall
    combinatorial_geometry = 'abs(z) < 1e-6'
    included_subdomains = 1006
    normal = '0 0 -1'
    new_sideset_name = inlet_interwrapper
  []

  [inlet_porous_flow_hfd]
    type = ParsedGenerateSideset
    input = inlet_interwrapper
    combinatorial_geometry = 'abs(z) < 1e-6'
    included_subdomains = 13
    normal = '0 0 -1'
    new_sideset_name = inlet_porous_flow_hfd
  []

  [inlet_porous_flow_p]
    type = ParsedGenerateSideset
    input = inlet_porous_flow_hfd
    combinatorial_geometry = 'abs(z) < 1e-6'
    included_subdomains = 14
    normal = '0 0 -1'
    new_sideset_name = inlet_porous_flow_p
  []

  [inlet_porous_flow_d1]
    type = ParsedGenerateSideset
    input = inlet_porous_flow_p
    combinatorial_geometry = 'abs(z) < 1e-6'
    included_subdomains = 15
    normal = '0 0 -1'
    new_sideset_name = inlet_porous_flow_d1
  []

  [inlet_porous_flow_d2]
    type = ParsedGenerateSideset
    input = inlet_porous_flow_d1
    combinatorial_geometry = 'abs(z) < 1e-6'
    included_subdomains = 16
    normal = '0 0 -1'
    new_sideset_name = inlet_porous_flow_d2
  []

  [inlet_porous_flow_k011]
    type = ParsedGenerateSideset
    input = inlet_porous_flow_d2
    combinatorial_geometry = 'abs(z) < 1e-6'
    included_subdomains = 17
    normal = '0 0 -1'
    new_sideset_name = inlet_porous_flow_k011
  []

  [inlet_porous_flow_x402]
    type = ParsedGenerateSideset
    input = inlet_porous_flow_k011
    combinatorial_geometry = 'abs(z) < 1e-6'
    included_subdomains = 18
    normal = '0 0 -1'
    new_sideset_name = inlet_porous_flow_x402
  []

  [inlet_central_assembly]
    type = ParsedGenerateSideset
    input = inlet_porous_flow_x402
    combinatorial_geometry = 'abs(z) < 1e-6'
    included_subdomains = '12'
    normal = '0 0 -1'
    new_sideset_name = inlet_central_assembly
  []

  [outlet_interwall]
    type = ParsedGenerateSideset
    input = inlet_central_assembly
    included_subdomains = '1004'
    combinatorial_geometry = 'abs(z - ${fparse height}) < 1e-6'
    normal = '0 0 1'
    new_sideset_name = outlet_interwall
  []

  [outlet_interwrapper]
    type = ParsedGenerateSideset
    input = outlet_interwall
    included_subdomains = '1006'
    combinatorial_geometry = 'abs(z - ${fparse height}) < 1e-6'
    normal = '0 0 1'
    new_sideset_name = outlet_interwrapper
  []

  [outlet_porous_flow]
    type = ParsedGenerateSideset
    input = outlet_interwrapper
    included_subdomains = '13 14 15 16 17 18'
    combinatorial_geometry = 'abs(z - ${fparse height}) < 1e-6'
    normal = '0 0 1'
    new_sideset_name = outlet_porous_flow
  []

  [outlet_central_assembly]
    type = ParsedGenerateSideset
    input = outlet_porous_flow
    included_subdomains = '12'
    combinatorial_geometry = 'abs(z - ${fparse height}) < 1e-6'
    normal = '0 0 1'
    new_sideset_name = outlet_central_assembly
  []

  [rename]
    type = RenameBlockGenerator
    input = outlet_central_assembly
    old_block = '1003 1004      1005 1006          12'
    new_block = 'wall interwall wall inter_wrapper center_porous_flow'
  []

  [rename2]
    type = RenameBlockGenerator
    input = rename
    old_block = '13              14            15             16             17               18'
    new_block = 'porous_flow_hfd porous_flow_p porous_flow_d1 porous_flow_d2 porous_flow_k011 porous_flow_x402'
  []

  [rotate]
    type = TransformGenerator
    input = rename2
    transform = ROTATE
    vector_value = '0 0 0'
  []

  # turn into meters
  [scale]
    type = TransformGenerator
    vector_value = '0.01 0.01 0.01'
    transform = SCALE
    input = rotate
  []
[]
