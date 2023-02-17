# a Pronghorn mesh for an EBRT-2 assembly

# sqrt(3) / 2 is by how much flat to flat is smaller than corer to corner
f = ${fparse sqrt(3) / 2}

# units are cm - do not forget to convert to meter
outer_duct_out = 5.8166
outer_duct_in = 5.5854
inner_duct_out = 4.8437
inner_duct_in = 4.64
height = 61.2
n_ax = 34

[Mesh]
  [fuel_center]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '4 4 4 4 4 4'
    background_intervals = 1
    background_block_ids = '12'
    polygon_size = ${fparse outer_duct_out / 2}
    duct_sizes = '${fparse inner_duct_in / f /2} ${fparse inner_duct_out / f /2} ${fparse outer_duct_in / f /2}'
    duct_intervals = '4    8    4'
    duct_block_ids = '1003 1004 1005'
    interface_boundary_names = 'inner_wall_in inner_wall_out outer_wall_in'
  []

  [extrude]
    type = AdvancedExtruderGenerator
    direction = '0 0 1'
    input = fuel_center
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

  [inlet_central_assembly]
    type = ParsedGenerateSideset
    input = inlet_interwall
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

  [outlet_central_assembly]
    type = ParsedGenerateSideset
    input = outlet_interwall
    included_subdomains = '12'
    combinatorial_geometry = 'abs(z - ${fparse height}) < 1e-6'
    normal = '0 0 1'
    new_sideset_name = outlet_central_assembly
  []

  [rename]
    type = RenameBlockGenerator
    input = outlet_central_assembly
    old_block = '1003 1004      1005 12'
    new_block = 'wall interwall wall center_porous_flow'
  []

  [rotate]
    type = TransformGenerator
    input = rename
    transform = ROTATE
    vector_value = '0 0 30'
  []

  [scale]
    type = TransformGenerator
    vector_value = '0.01 0.01 0.01'
    transform = SCALE
    input = rotate
  []
[]
