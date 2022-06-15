# following Advanced Burner Test Reactor Preconceptual Design Report

# A core model mesh of the ABTR

# sqrt(3) / 2 is by how much flat to flat is smaller than corer to corner
f = ${fparse sqrt(3) / 2}

# units are cm - do not forget to convert to meter
fuel_element_pitch = 14.598
inter_assembly_gap = 0.4
duct_thickness = 0.3
height = 260
top_bottom = 40
fuel_pin_pitch = 0.904
control_pin_pitch = 1.2476

orifice_plate_height = 5

duct_outside = ${fparse fuel_element_pitch - inter_assembly_gap}
duct_inside = ${fparse duct_outside - 2 * duct_thickness}

# distance between bottom and fuel pins and control pins
withdrawn_distance = 80

# height of loss layer at CR interface
loss_layer_height = 5

# total height of the geometry
total_height = ${fparse orifice_plate_height + height + 2 * top_bottom}

[Mesh]
  [fuel]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '2 2 2 2 2 2'
    background_intervals = 1
    background_block_ids = '1'
    # note that polygon_size is "like radius"
    polygon_size = ${fparse fuel_element_pitch / 2}
    duct_sizes = '${fparse duct_inside / 2 / f}'
    duct_intervals = '2'
    duct_block_ids = '2'
    interface_boundary_names = 'remove1'
  []

  [control]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '2 2 2 2 2 2'
    background_intervals = 1
    background_block_ids = '11'
    # note that polygon_size is "like radius"
    polygon_size = ${fparse fuel_element_pitch / 2}
    duct_sizes = '${fparse duct_inside / 2 / f}'
    duct_intervals = '2'
    duct_block_ids = '2'
    interface_boundary_names = 'remove2'
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
    inputs = 'fuel control dummy'
    pattern = '
               0 0 0 0;
              0 0 0 0 0;
             0 1 0 0 1 0;
            0 0 0 1 0 0 0;
             0 0 0 0 0 0;
              0 0 1 0 0;
               0 0 0 0'
    pattern_boundary = hexagon
    background_block_id = 1000
    hexagon_size_style = apothem
    hexagon_size = 60
    external_boundary_id = 10000
  []

  [extrude]
    type = FancyExtruderGenerator
    direction = '0 0 1'
    # first line: cold pool
    # second line: orifice plate
    # third line: unrodded portion of CR assemblies
    # fourth line: loss layer for the control element model
    # fifth line: rodded portion of CR
    # sixth line: hot pool
    heights = '${fparse top_bottom - orifice_plate_height} ${orifice_plate_height}
               ${orifice_plate_height}
               ${orifice_plate_height} ${fparse withdrawn_distance - orifice_plate_height - 2 * loss_layer_height} ${loss_layer_height}
               ${loss_layer_height}
               ${loss_layer_height} ${fparse height - loss_layer_height - withdrawn_distance}
               ${top_bottom}'
    # num_layers = '6 5
    #               10
    #               5 10 5
    #               10
    #               5 20
    #               6'
    num_layers = '3 2
                 5
                 2 5 2
                 5
                 2 10
                 3'
    subdomain_swaps = '1 101 2 102 11 101; 1 101 2 102 11 101;
                       1 301 11 302 1000 2000;
                       11 402 1000 2000; 11 402 1000 2000; 11 402 1000 2000;
                       11 502 1000 2000;
                       1000 2000; 1000 2000;
                       1 201 2 202 11 201 1000 2000'
    input = pattern
  []

  [name_blocks]
    type = RenameBlockGenerator
    old_block = '1
                 2
                 11
                 402
                 101
                 102
                 201
                 202
                 502
                 301
                 302
                 1000'
    new_block = 'fuel
                 wrapper_interwrapper
                 control
                 free_duct
                 cold_pool1
                 cold_pool2
                 hot_pool1
                 hot_pool2
                 cr_tip
                 fuel_orifice
                 control_orifice
                 inlet_duct'
    input = extrude
  []

  [inlet]
    type = ParsedGenerateSideset
    combinatorial_geometry = 'abs(z - ${top_bottom}) < 1e-5'
    included_subdomain_ids = '1000'
    new_sideset_name = inlet
    input = name_blocks
  []

  [cold_pool_walls1]
    type = ParsedGenerateSideset
    combinatorial_geometry = 'abs(z) < 1e-5'
    new_sideset_name = cold_pool_wall
    input = inlet
  []

  [cold_pool_walls2]
    type = RenameBoundaryGenerator
    old_boundary = 10000
    new_boundary = cold_pool_wall
    input = cold_pool_walls1
  []

  [outlet]
    type = ParsedGenerateSideset
    combinatorial_geometry = 'abs(z - ${total_height}) < 1e-5'
    included_subdomain_ids = '201 202'
    new_sideset_name = outlet
    input = cold_pool_walls2
  []

  [outer_perimeter]
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = 'hot_pool1 hot_pool2'
    paired_block = '2000'
    new_boundary = 'hot_pool_wall'
    input = outlet
  []

  [interior_walls]
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = 'fuel
                     control
                     free_duct
                     cold_pool1
                     cold_pool2
                     hot_pool1
                     hot_pool2
                     cr_tip
                     fuel_orifice
                     control_orifice'
    paired_block = 'wrapper_interwrapper'
    new_boundary = interior_walls
    input = outer_perimeter
  []

  [scale]
    type = TransformGenerator
    vector_value = '0.01 0.01 0.01'
    transform = SCALE
    input = interior_walls
  []

  [delete2000]
    type = BlockDeletionGenerator
    block = 2000
    input = scale
  []
[]
