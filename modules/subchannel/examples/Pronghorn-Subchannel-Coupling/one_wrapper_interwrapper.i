# a wrapper mesh for coupling to subchannel11

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
  [fuel_center]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '2 2 2 2 2 2'
    background_intervals = 1
    background_block_ids = '12'
    polygon_size = ${fparse fuel_element_pitch / 2}
    duct_sizes = '${fparse duct_inside / 2 / f} ${fparse duct_outside / 2 / f}'
    duct_intervals = '4    2'
    duct_block_ids = '1003 1004'
    outward_interface_boundary_names = 'inner_wall outer_wall'
  []

  [extrude]
    type = AdvancedExtruderGenerator
    direction = '0 0 1'
    input = fuel_center
    heights = '${height}'
    num_layers = '${n_ax}'
  []

  [inlet_interwrapper]
    type = ParsedGenerateSideset
    input = extrude
    combinatorial_geometry = 'abs(z) < 1e-6'
    included_subdomains = 1004
    normal = '0 0 -1'
    new_sideset_name = inlet_interwrapper
  []

  [inlet_central_assembly]
    type = ParsedGenerateSideset
    input = inlet_interwrapper
    combinatorial_geometry = 'abs(z) < 1e-6'
    included_subdomains = '12'
    normal = '0 0 -1'
    new_sideset_name = inlet_central_assembly
  []

  [outlet_boundary]
    type = ParsedGenerateSideset
    input = inlet_central_assembly
    included_subdomains = '1004 12'
    combinatorial_geometry = 'abs(z - ${fparse height}) < 1e-6'
    normal = '0 0 1'
    new_sideset_name = outlet
  []

  [outlet_central_assembly]
    type = ParsedGenerateSideset
    input = outlet_boundary
    included_subdomains = '12'
    combinatorial_geometry = 'abs(z - ${fparse height}) < 1e-6'
    normal = '0 0 1'
    new_sideset_name = outlet_central_assembly
  []

  [rename]
    type = RenameBlockGenerator
    input = outlet_central_assembly
    old_block = '1003    1004         12'
    new_block = 'wrapper interwrapper center_porous_flow'
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
