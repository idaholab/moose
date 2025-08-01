# a MOOSE mesh for 19 ABR assemblies
# sqrt(3) / 2 is by how much flat to flat is smaller than corer to corner
f = ${fparse sqrt(3) / 2}

# units are cm - do not forget to convert to meter
outer_duct_out = 15.8123 # outer size of the hexagonal duct (side to side)
outer_duct_in = 15.0191 # flat to flat
inter_wrapper_width = 0.4348 # not change
height = 480.2

# discretization
n_ax = 50
ns = 10
duct_intervals_center = '4 6 4 3'
duct_intervals_perishperic = '4 4'

[Mesh]
  [XX00]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '${ns} ${ns} ${ns} ${ns} ${ns} ${ns}'
    background_intervals = 1
    background_block_ids = '12'
    polygon_size = ${fparse outer_duct_out / 2 + inter_wrapper_width / 2}
    duct_sizes = '${fparse outer_duct_in / f /2} ${fparse outer_duct_out / f / 2}'
    duct_intervals = ${duct_intervals_perishperic}
    duct_block_ids = '1003 1006'
    outward_interface_boundary_names = 'wall_in_00 wall_out_00'
    interface_boundary_id_shift = 100
  []

  [XX01]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '${ns} ${ns} ${ns} ${ns} ${ns} ${ns}'
    background_intervals = 1
    background_block_ids = '13'
    polygon_size = ${fparse outer_duct_out / 2 + inter_wrapper_width / 2}
    duct_sizes = '${fparse outer_duct_in / f /2} ${fparse outer_duct_out / f / 2}'
    duct_intervals = ${duct_intervals_perishperic}
    duct_block_ids = '1003  1006'
    outward_interface_boundary_names = 'wall_in_01 wall_out_01'
    interface_boundary_id_shift = 200
  []

  [XX02]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '${ns} ${ns} ${ns} ${ns} ${ns} ${ns}'
    background_intervals = 1
    background_block_ids = '14'
    polygon_size = ${fparse outer_duct_out / 2 + inter_wrapper_width / 2}
    duct_sizes = '${fparse outer_duct_in / f /2} ${fparse outer_duct_out / f / 2}'
    duct_intervals = ${duct_intervals_perishperic}
    duct_block_ids = '1003  1006'
    outward_interface_boundary_names = 'wall_in_02 wall_out_02'
    interface_boundary_id_shift = 300
  []

  [XX03]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '${ns} ${ns} ${ns} ${ns} ${ns} ${ns}'
    background_intervals = 1
    background_block_ids = '15'
    polygon_size = ${fparse outer_duct_out / 2 + inter_wrapper_width / 2}
    duct_sizes = '${fparse outer_duct_in / f /2} ${fparse outer_duct_out / f / 2}'
    duct_intervals = ${duct_intervals_perishperic}
    duct_block_ids = '1003  1006'
    outward_interface_boundary_names = 'wall_in_03 wall_out_03'
    interface_boundary_id_shift = 400
  []

  [XX04]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '${ns} ${ns} ${ns} ${ns} ${ns} ${ns}'
    background_intervals = 1
    background_block_ids = '16'
    polygon_size = ${fparse outer_duct_out / 2 + inter_wrapper_width / 2}
    duct_sizes = '${fparse outer_duct_in / f /2} ${fparse outer_duct_out / f / 2}'
    duct_intervals = ${duct_intervals_perishperic}
    duct_block_ids = '1003  1006'
    outward_interface_boundary_names = 'wall_in_04 wall_out_04'
    interface_boundary_id_shift = 500
  []

  [XX05]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '${ns} ${ns} ${ns} ${ns} ${ns} ${ns}'
    background_intervals = 1
    background_block_ids = '17'
    polygon_size = ${fparse outer_duct_out / 2 + inter_wrapper_width / 2}
    duct_sizes = '${fparse outer_duct_in / f /2} ${fparse outer_duct_out / f / 2}'
    duct_intervals = ${duct_intervals_perishperic}
    duct_block_ids = '1003  1006'
    outward_interface_boundary_names = 'wall_in_05 wall_out_05'
    interface_boundary_id_shift = 600
  []

  [XX06]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '${ns} ${ns} ${ns} ${ns} ${ns} ${ns}'
    background_intervals = 1
    background_block_ids = '18'
    polygon_size = ${fparse outer_duct_out / 2 + inter_wrapper_width / 2}
    duct_sizes = '${fparse outer_duct_in / f /2} ${fparse outer_duct_out / f / 2}'
    duct_intervals = ${duct_intervals_perishperic}
    duct_block_ids = '1003  1006'
    outward_interface_boundary_names = 'wall_in_06 wall_out_06'
    interface_boundary_id_shift = 700
  []

  [XX07]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '${ns} ${ns} ${ns} ${ns} ${ns} ${ns}'
    background_intervals = 1
    background_block_ids = '19'
    polygon_size = ${fparse outer_duct_out / 2 + inter_wrapper_width / 2}
    duct_sizes = '${fparse outer_duct_in / f /2} ${fparse outer_duct_out / f / 2}'
    duct_intervals = ${duct_intervals_perishperic}
    duct_block_ids = '1003  1006'
    outward_interface_boundary_names = 'wall_in_07 wall_out_07'
    interface_boundary_id_shift = 800
  []

  [XX08]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '${ns} ${ns} ${ns} ${ns} ${ns} ${ns}'
    background_intervals = 1
    background_block_ids = '20'
    polygon_size = ${fparse outer_duct_out / 2 + inter_wrapper_width / 2}
    duct_sizes = '${fparse outer_duct_in / f /2} ${fparse outer_duct_out / f / 2}'
    duct_intervals = ${duct_intervals_perishperic}
    duct_block_ids = '1003  1006'
    outward_interface_boundary_names = 'wall_in_08 wall_out_08'
    interface_boundary_id_shift = 900
  []

  [XX09]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '${ns} ${ns} ${ns} ${ns} ${ns} ${ns}'
    background_intervals = 1
    background_block_ids = '21'
    polygon_size = ${fparse outer_duct_out / 2 + inter_wrapper_width / 2}
    duct_sizes = '${fparse outer_duct_in / f /2} ${fparse outer_duct_out / f / 2}'
    duct_intervals = ${duct_intervals_perishperic}
    duct_block_ids = '1003  1006'
    outward_interface_boundary_names = 'wall_in_09 wall_out_09'
    interface_boundary_id_shift = 1000
  []

  [XX10]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '${ns} ${ns} ${ns} ${ns} ${ns} ${ns}'
    background_intervals = 1
    background_block_ids = '22'
    polygon_size = ${fparse outer_duct_out / 2 + inter_wrapper_width / 2}
    duct_sizes = '${fparse outer_duct_in / f /2} ${fparse outer_duct_out / f / 2}'
    duct_intervals = ${duct_intervals_perishperic}
    duct_block_ids = '1003  1006'
    outward_interface_boundary_names = 'wall_in_10 wall_out_10'
    interface_boundary_id_shift = 1100
  []

  [XX11]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '${ns} ${ns} ${ns} ${ns} ${ns} ${ns}'
    background_intervals = 1
    background_block_ids = '23'
    polygon_size = ${fparse outer_duct_out / 2 + inter_wrapper_width / 2}
    duct_sizes = '${fparse outer_duct_in / f /2} ${fparse outer_duct_out / f / 2}'
    duct_intervals = ${duct_intervals_perishperic}
    duct_block_ids = '1003  1006'
    outward_interface_boundary_names = 'wall_in_11 wall_out_11'
    interface_boundary_id_shift = 1200
  []

  [XX12]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '${ns} ${ns} ${ns} ${ns} ${ns} ${ns}'
    background_intervals = 1
    background_block_ids = '24'
    polygon_size = ${fparse outer_duct_out / 2 + inter_wrapper_width / 2}
    duct_sizes = '${fparse outer_duct_in / f /2} ${fparse outer_duct_out / f / 2}'
    duct_intervals = ${duct_intervals_perishperic}
    duct_block_ids = '1003  1006'
    outward_interface_boundary_names = 'wall_in_12 wall_out_12'
    interface_boundary_id_shift = 1300
  []

  [XX13]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '${ns} ${ns} ${ns} ${ns} ${ns} ${ns}'
    background_intervals = 1
    background_block_ids = '25'
    polygon_size = ${fparse outer_duct_out / 2 + inter_wrapper_width / 2}
    duct_sizes = '${fparse outer_duct_in / f /2} ${fparse outer_duct_out / f / 2}'
    duct_intervals = ${duct_intervals_perishperic}
    duct_block_ids = '1003  1006'
    outward_interface_boundary_names = 'wall_in_13 wall_out_13'
    interface_boundary_id_shift = 1400
  []

  [XX14]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '${ns} ${ns} ${ns} ${ns} ${ns} ${ns}'
    background_intervals = 1
    background_block_ids = '26'
    polygon_size = ${fparse outer_duct_out / 2 + inter_wrapper_width / 2}
    duct_sizes = '${fparse outer_duct_in / f /2} ${fparse outer_duct_out / f / 2}'
    duct_intervals = ${duct_intervals_perishperic}
    duct_block_ids = '1003  1006'
    outward_interface_boundary_names = 'wall_in_14 wall_out_14'
    interface_boundary_id_shift = 1500
  []

  [XX15]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '${ns} ${ns} ${ns} ${ns} ${ns} ${ns}'
    background_intervals = 1
    background_block_ids = '27'
    polygon_size = ${fparse outer_duct_out / 2 + inter_wrapper_width / 2}
    duct_sizes = '${fparse outer_duct_in / f /2} ${fparse outer_duct_out / f / 2}'
    duct_intervals = ${duct_intervals_perishperic}
    duct_block_ids = '1003  1006'
    outward_interface_boundary_names = 'wall_in_15 wall_out_15'
    interface_boundary_id_shift = 1600
  []

  [XX16]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '${ns} ${ns} ${ns} ${ns} ${ns} ${ns}'
    background_intervals = 1
    background_block_ids = '28'
    polygon_size = ${fparse outer_duct_out / 2 + inter_wrapper_width / 2}
    duct_sizes = '${fparse outer_duct_in / f /2} ${fparse outer_duct_out / f / 2}'
    duct_intervals = ${duct_intervals_perishperic}
    duct_block_ids = '1003  1006'
    outward_interface_boundary_names = 'wall_in_16 wall_out_16'
    interface_boundary_id_shift = 1700
  []

  [XX17]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '${ns} ${ns} ${ns} ${ns} ${ns} ${ns}'
    background_intervals = 1
    background_block_ids = '29'
    polygon_size = ${fparse outer_duct_out / 2 + inter_wrapper_width / 2}
    duct_sizes = '${fparse outer_duct_in / f /2} ${fparse outer_duct_out / f / 2}'
    duct_intervals = ${duct_intervals_perishperic}
    duct_block_ids = '1003  1006'
    outward_interface_boundary_names = 'wall_in_17 wall_out_17'
    interface_boundary_id_shift = 1800
  []

  [XX18]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '${ns} ${ns} ${ns} ${ns} ${ns} ${ns}'
    background_intervals = 1
    background_block_ids = '30'
    polygon_size = ${fparse outer_duct_out / 2 + inter_wrapper_width / 2}
    duct_sizes = '${fparse outer_duct_in / f /2} ${fparse outer_duct_out / f / 2}'
    duct_intervals = ${duct_intervals_perishperic}
    duct_block_ids = '1003  1006'
    outward_interface_boundary_names = 'wall_in_18 wall_out_18'
    interface_boundary_id_shift = 1900
  []

  [pattern]
    type = PatternedHexMeshGenerator
    inputs = 'XX00 XX01 XX02 XX03 XX04 XX05 XX06 XX07 XX08 XX09
              XX10 XX11 XX12 XX13 XX14 XX15 XX16 XX17 XX18'
    pattern =
            ' 15 14 13;
             16 5 4 12;
            17 6 0 3 11;
             18 1 2 10;
               7 8 9'
    pattern_boundary = none
  []

  [extrude]
    type = AdvancedExtruderGenerator
    direction = '0 0 1'
    input = pattern
    heights = '${height}'
    num_layers = '${n_ax}'
  []

  [inlet_interwrapper]
    type = ParsedGenerateSideset
    input = extrude
    combinatorial_geometry = 'abs(z) < 1e-6'
    included_subdomains = '1006 1003'
    normal = '0 0 -1'
    new_sideset_name = inlet_interwrapper
  []

  [inlet_porous_flow_hfd]
    type = ParsedGenerateSideset
    input = inlet_interwrapper
    combinatorial_geometry = 'abs(z) < 1e-6'
    included_subdomains = 13
    normal = '0 0 -1'
    new_sideset_name = inlet_porous_flow_01
  []

  [inlet_porous_flow_p]
    type = ParsedGenerateSideset
    input = inlet_porous_flow_hfd
    combinatorial_geometry = 'abs(z) < 1e-6'
    included_subdomains = 14
    normal = '0 0 -1'
    new_sideset_name = inlet_porous_flow_02
  []

  [inlet_porous_flow_d1]
    type = ParsedGenerateSideset
    input = inlet_porous_flow_p
    combinatorial_geometry = 'abs(z) < 1e-6'
    included_subdomains = 15
    normal = '0 0 -1'
    new_sideset_name = inlet_porous_flow_03
  []

  [inlet_porous_flow_d2]
    type = ParsedGenerateSideset
    input = inlet_porous_flow_d1
    combinatorial_geometry = 'abs(z) < 1e-6'
    included_subdomains = 16
    normal = '0 0 -1'
    new_sideset_name = inlet_porous_flow_04
  []

  [inlet_porous_flow_k011]
    type = ParsedGenerateSideset
    input = inlet_porous_flow_d2
    combinatorial_geometry = 'abs(z) < 1e-6'
    included_subdomains = 17
    normal = '0 0 -1'
    new_sideset_name = inlet_porous_flow_05
  []

  [inlet_porous_flow_x402]
    type = ParsedGenerateSideset
    input = inlet_porous_flow_k011
    combinatorial_geometry = 'abs(z) < 1e-6'
    included_subdomains = 18
    normal = '0 0 -1'
    new_sideset_name = inlet_porous_flow_06
  []

  [inlet_central_assembly]
    type = ParsedGenerateSideset
    input = inlet_porous_flow_x402
    combinatorial_geometry = 'abs(z) < 1e-6'
    included_subdomains = '12'
    normal = '0 0 -1'
    new_sideset_name = inlet_porous_flow_00
  []

  [inlet_porous_flow_07]
    type = ParsedGenerateSideset
    input = inlet_central_assembly
    combinatorial_geometry = 'abs(z) < 1e-6'
    included_subdomains = '19'
    normal = '0 0 -1'
    new_sideset_name = inlet_porous_flow_07
  []

  [inlet_porous_flow_08]
    type = ParsedGenerateSideset
    input = inlet_porous_flow_07
    combinatorial_geometry = 'abs(z) < 1e-6'
    included_subdomains = '20'
    normal = '0 0 -1'
    new_sideset_name = inlet_porous_flow_08
  []

  [inlet_porous_flow_09]
    type = ParsedGenerateSideset
    input = inlet_porous_flow_08
    combinatorial_geometry = 'abs(z) < 1e-6'
    included_subdomains = '21'
    normal = '0 0 -1'
    new_sideset_name = inlet_porous_flow_09
  []

  [inlet_porous_flow_10]
    type = ParsedGenerateSideset
    input = inlet_porous_flow_09
    combinatorial_geometry = 'abs(z) < 1e-6'
    included_subdomains = '22'
    normal = '0 0 -1'
    new_sideset_name = inlet_porous_flow_10
  []

  [inlet_porous_flow_11]
    type = ParsedGenerateSideset
    input = inlet_porous_flow_10
    combinatorial_geometry = 'abs(z) < 1e-6'
    included_subdomains = '23'
    normal = '0 0 -1'
    new_sideset_name = inlet_porous_flow_11
  []

  [inlet_porous_flow_12]
    type = ParsedGenerateSideset
    input = inlet_porous_flow_11
    combinatorial_geometry = 'abs(z) < 1e-6'
    included_subdomains = '24'
    normal = '0 0 -1'
    new_sideset_name = inlet_porous_flow_12
  []

  [inlet_porous_flow_13]
    type = ParsedGenerateSideset
    input = inlet_porous_flow_12
    combinatorial_geometry = 'abs(z) < 1e-6'
    included_subdomains = '25'
    normal = '0 0 -1'
    new_sideset_name = inlet_porous_flow_13
  []

  [inlet_porous_flow_14]
    type = ParsedGenerateSideset
    input = inlet_porous_flow_13
    combinatorial_geometry = 'abs(z) < 1e-6'
    included_subdomains = '26'
    normal = '0 0 -1'
    new_sideset_name = inlet_porous_flow_14
  []

  [inlet_porous_flow_15]
    type = ParsedGenerateSideset
    input = inlet_porous_flow_14
    combinatorial_geometry = 'abs(z) < 1e-6'
    included_subdomains = '27'
    normal = '0 0 -1'
    new_sideset_name = inlet_porous_flow_15
  []

  [inlet_porous_flow_16]
    type = ParsedGenerateSideset
    input = inlet_porous_flow_15
    combinatorial_geometry = 'abs(z) < 1e-6'
    included_subdomains = '28'
    normal = '0 0 -1'
    new_sideset_name = inlet_porous_flow_16
  []

  [inlet_porous_flow_17]
    type = ParsedGenerateSideset
    input = inlet_porous_flow_16
    combinatorial_geometry = 'abs(z) < 1e-6'
    included_subdomains = '29'
    normal = '0 0 -1'
    new_sideset_name = inlet_porous_flow_17
  []

  [inlet_porous_flow_18]
    type = ParsedGenerateSideset
    input = inlet_porous_flow_17
    combinatorial_geometry = 'abs(z) < 1e-6'
    included_subdomains = '30'
    normal = '0 0 -1'
    new_sideset_name = inlet_porous_flow_18
  []

  [outlet_interwrapper]
    type = ParsedGenerateSideset
    input = inlet_porous_flow_18 # outlet_interwall
    included_subdomains = '1006 1003'
    combinatorial_geometry = 'abs(z - ${fparse height}) < 1e-6'
    normal = '0 0 1'
    new_sideset_name = outlet_interwrapper
  []

  [outlet_porous_flow]
    type = ParsedGenerateSideset
    input = outlet_interwrapper
    included_subdomains = '13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30'
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
    new_sideset_name = outlet_porous_flow_00
  []

  [rename]
    type = RenameBlockGenerator
    input = outlet_central_assembly
    old_block = '1003     1006          12'
    new_block = 'wall inter_wrapper porous_flow_00'
  []

  [rename2]
    type = RenameBlockGenerator
    input = rename
    old_block = '13              14            15             16             17               18
                 19              20            21             22             23               24
                 25              26            27             28             29               30'
    new_block = 'porous_flow_01 porous_flow_02 porous_flow_03 porous_flow_04 porous_flow_05 porous_flow_06
                 porous_flow_07 porous_flow_08 porous_flow_09 porous_flow_10 porous_flow_11 porous_flow_12
                 porous_flow_13 porous_flow_14 porous_flow_15 porous_flow_16 porous_flow_17 porous_flow_18'
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

  [new_wall_boundary_00]
    type = SideSetsBetweenSubdomainsGenerator
    input = scale
    new_boundary = 'prsb_interface_00'
    primary_block = 'wall'
    paired_block = 'porous_flow_00'
  []

  [new_wall_boundary_01]
    type = SideSetsBetweenSubdomainsGenerator
    input = new_wall_boundary_00
    new_boundary = 'prsb_interface_01'
    primary_block = 'wall'
    paired_block = 'porous_flow_01'
  []

  [new_wall_boundary_02]
    type = SideSetsBetweenSubdomainsGenerator
    input = new_wall_boundary_01
    new_boundary = 'prsb_interface_02'
    primary_block = 'wall'
    paired_block = 'porous_flow_02'
  []

  [new_wall_boundary_03]
    type = SideSetsBetweenSubdomainsGenerator
    input = new_wall_boundary_02
    new_boundary = 'prsb_interface_03'
    primary_block = 'wall'
    paired_block = 'porous_flow_03'
  []

  [new_wall_boundary_04]
    type = SideSetsBetweenSubdomainsGenerator
    input = new_wall_boundary_03
    new_boundary = 'prsb_interface_04'
    primary_block = 'wall'
    paired_block = 'porous_flow_04'
  []

  [new_wall_boundary_05]
    type = SideSetsBetweenSubdomainsGenerator
    input = new_wall_boundary_04
    new_boundary = 'prsb_interface_05'
    primary_block = 'wall'
    paired_block = 'porous_flow_05'
  []

  [new_wall_boundary_06]
    type = SideSetsBetweenSubdomainsGenerator
    input = new_wall_boundary_05
    new_boundary = 'prsb_interface_06'
    primary_block = 'wall'
    paired_block = 'porous_flow_06'
  []

  [new_wall_boundary_07]
    type = SideSetsBetweenSubdomainsGenerator
    input = new_wall_boundary_06
    new_boundary = 'prsb_interface_07'
    primary_block = 'wall'
    paired_block = 'porous_flow_07'
  []

  [new_wall_boundary_08]
    type = SideSetsBetweenSubdomainsGenerator
    input = new_wall_boundary_07
    new_boundary = 'prsb_interface_08'
    primary_block = 'wall'
    paired_block = 'porous_flow_08'
  []

  [new_wall_boundary_09]
    type = SideSetsBetweenSubdomainsGenerator
    input = new_wall_boundary_08
    new_boundary = 'prsb_interface_09'
    primary_block = 'wall'
    paired_block = 'porous_flow_09'
  []

  [new_wall_boundary_10]
    type = SideSetsBetweenSubdomainsGenerator
    input = new_wall_boundary_09
    new_boundary = 'prsb_interface_10'
    primary_block = 'wall'
    paired_block = 'porous_flow_10'
  []

  [new_wall_boundary_11]
    type = SideSetsBetweenSubdomainsGenerator
    input = new_wall_boundary_10
    new_boundary = 'prsb_interface_11'
    primary_block = 'wall'
    paired_block = 'porous_flow_11'
  []

  [new_wall_boundary_12]
    type = SideSetsBetweenSubdomainsGenerator
    input = new_wall_boundary_11
    new_boundary = 'prsb_interface_12'
    primary_block = 'wall'
    paired_block = 'porous_flow_12'
  []

  [new_wall_boundary_13]
    type = SideSetsBetweenSubdomainsGenerator
    input = new_wall_boundary_12
    new_boundary = 'prsb_interface_13'
    primary_block = 'wall'
    paired_block = 'porous_flow_13'
  []

  [new_wall_boundary_14]
    type = SideSetsBetweenSubdomainsGenerator
    input = new_wall_boundary_13
    new_boundary = 'prsb_interface_14'
    primary_block = 'wall'
    paired_block = 'porous_flow_14'
  []

  [new_wall_boundary_15]
    type = SideSetsBetweenSubdomainsGenerator
    input = new_wall_boundary_14
    new_boundary = 'prsb_interface_15'
    primary_block = 'wall'
    paired_block = 'porous_flow_15'
  []

  [new_wall_boundary_16]
    type = SideSetsBetweenSubdomainsGenerator
    input = new_wall_boundary_15
    new_boundary = 'prsb_interface_16'
    primary_block = 'wall'
    paired_block = 'porous_flow_16'
  []

  [new_wall_boundary_17]
    type = SideSetsBetweenSubdomainsGenerator
    input = new_wall_boundary_16
    new_boundary = 'prsb_interface_17'
    primary_block = 'wall'
    paired_block = 'porous_flow_17'
  []

  [new_wall_boundary_18]
    type = SideSetsBetweenSubdomainsGenerator
    input = new_wall_boundary_17
    new_boundary = 'prsb_interface_18'
    primary_block = 'wall'
    paired_block = 'porous_flow_18'
  []

  [delete_assembly]
    type = BlockDeletionGenerator
    block = '12 13 14 15 16 17 18 19 20 21
             22 23 24 25 26 27 28 29 30'
    input = 'new_wall_boundary_18'
  []
[]
