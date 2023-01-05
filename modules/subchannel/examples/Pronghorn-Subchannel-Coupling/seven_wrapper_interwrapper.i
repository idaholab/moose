# a wrapper mesh for coupling to subchannel

# sqrt(3) / 2 is by how much flat to flat is smaller than corer to corner
f = ${fparse sqrt(3) / 2}

# units are cm - do not forget to convert to meter
fuel_element_pitch = 14.598
inter_assembly_gap = 0.4
duct_thickness = 0.3
top_bottom = 40
fuel_pin_pitch = 0.904
control_pin_pitch = 1.2476

orifice_plate_height = 5

duct_outside = ${fparse fuel_element_pitch - inter_assembly_gap}
duct_inside = ${fparse duct_outside - 2 * duct_thickness}
fuel_last_row_pins = ${fparse 16 * fuel_pin_pitch * f}

height = 260
n_ax = 25

[Mesh]
  [fuel]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '2 2 2 2 2 2'
    background_intervals = 2
    background_block_ids = '1 2'
    # note that polygon_size is "like radius"
    polygon_size = ${fparse fuel_element_pitch / 2}
    duct_sizes = '${fparse fuel_last_row_pins / 2 / f} ${fparse duct_inside / 2 / f} ${fparse duct_outside / 2 / f}'
    duct_intervals = '2 4    2'
    duct_block_ids = '3 1003 1004'
    interface_boundary_names = 'remove1 remove2 remove3'
  []

  [fuel_out]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '2 2 2 2 2 2'
    background_intervals = 1
    background_block_ids = '11'
    # note that polygon_size is "like radius"
    polygon_size = ${fparse fuel_element_pitch / 2}
    duct_sizes = '${fparse duct_inside / 2 / f} ${fparse duct_outside / 2 / f}'
    duct_intervals = '4    2'
    duct_block_ids = '1003 1004'
    interface_boundary_names = 'remove4 remove5'
  []

  [dummy]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '2 2 2 2 2 2'
    background_intervals = 1
    background_block_ids = 1000
    # note that polygon_size is "like radius"
    polygon_size = ${fparse fuel_element_pitch / 2}
  []

  [pattern]
    type = PatternedHexMeshGenerator
    inputs = 'fuel dummy fuel_out'
    pattern = '1 1 1;
              1 2 2 1;
             1 2 0 2 1;
              1 2 2 1;
               1 1 1'
    pattern_boundary = none
  []

  [extrude]
    type = FancyExtruderGenerator
    direction = '0 0 1'
    input = pattern
    heights = '${height}'
    num_layers = '${n_ax}'
  []

  [inlet_boundary_interwrapper]
    type = ParsedGenerateSideset
    input = extrude
    combinatorial_geometry = 'abs(z) < 1e-6'
    included_subdomains = 1004
    normal = '0 0 -1'
    new_sideset_name = inlet_interwrapper
  []

  [inlet_boundary_assembly]
    type = ParsedGenerateSideset
    input = inlet_boundary_interwrapper
    combinatorial_geometry = 'abs(z) < 1e-6'
    included_subdomains = 11
    normal = '0 0 -1'
    new_sideset_name = inlet_assembly
  []

  [outlet_boundary]
    type = ParsedGenerateSideset
    input = inlet_boundary_assembly
    included_subdomains = '1004 11'
    combinatorial_geometry = 'abs(z - ${fparse height}) < 1e-6'
    normal = '0 0 1'
    new_sideset_name = outlet
  []

  [outside_circumference]
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = '1004'
    paired_block = '1000'
    new_boundary = outside_circumference
    input = outlet_boundary
  []

  [inner_wall]
    type = SideSetsBetweenSubdomainsGenerator
    input = outside_circumference
    primary_block = '1003'
    paired_block = '3'
    new_boundary = inner_wall
  []

  [conjugate_heat_transfer]
    type = SideSetsBetweenSubdomainsGenerator
    input = inner_wall
    primary_block = '1004 11'
    paired_block = '1003'
    new_boundary = conjugate_heat_transfer_boundary
  []

  [remove]
    type = BlockDeletionGenerator
    block = '1 2 3 1000'
    input = conjugate_heat_transfer
  []

  [rename]
    type = RenameBlockGenerator
    input = remove
    old_block = '1003    1004         11'
    new_block = 'wrapper interwrapper porous_flow'
  []

  [rotate]
    type = TransformGenerator
    input = rename
    transform = ROTATE
    vector_value = '0 0 0'
  []

  [scale]
    type = TransformGenerator
    vector_value = '0.01 0.01 0.01'
    transform = SCALE
    input = rotate
  []
[]
