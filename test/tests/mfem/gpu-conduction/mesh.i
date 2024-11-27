pin_radius = 1
hp_radius = 1.1
gap_thickness = 0.05
pin_pitch = 3.4
asy_pitch = 32
monolith_pitch = 150
canister_diameter = 224
drum_radius = 15
poison_thickness = 1

# Block and material IDs
uo2_11 = 1
uo2_12 = 2
uo2_13 = 3
uo2_21 = 4
uo2_22 = 5
uo2_23 = 6
monolith_11 = 7
monolith_12 = 8
monolith_13 = 9
monolith_21 = 10
monolith_22 = 11
monolith_23 = 12
monolith_lr = 13
monolith_ur = 14
monolith_rr = 15
monolith_cr = 16
htpipe = 17
absorber = 18
strip = 181
gap = 19
gap_uo2 = 191
gap_htpipe = 192
gap_reflector = 193
gap_drum = 194
reflector = 20

uo2_11_tri = 100
uo2_12_tri = 200
uo2_13_tri = 300
uo2_21_tri = 400
uo2_22_tri = 500
uo2_23_tri = 600
monolith_lr_tri = 1300
monolith_ur_tri = 1400
monolith_rr_tri = 1500
monolith_cr_tri = 1600
reflector_tri = 2000

[Mesh]
  [fuel_inner]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '2 2 2 2 2 2'
    background_intervals = 1
    background_block_ids = '${monolith_11}'
    polygon_size = ${fparse pin_pitch / 2}
    polygon_size_style = 'apothem'
    ring_id_name = 'ring_id'
    ring_id_assign_type = ring_wise
    ring_radii = '${pin_radius} ${fparse pin_radius + gap_thickness}'
    ring_intervals = '3 1'
    ring_block_ids = '${uo2_11_tri} ${uo2_11} ${gap_uo2}'
  []
  [fuel_outer]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '2 2 2 2 2 2'
    background_intervals = 1
    background_block_ids = '${monolith_21}'
    polygon_size = ${fparse pin_pitch / 2}
    polygon_size_style = 'apothem'
    ring_id_name = 'ring_id'
    ring_id_assign_type = ring_wise
    ring_radii = '${pin_radius} ${fparse pin_radius + gap_thickness}'
    ring_intervals = '3 1'
    ring_block_ids = '${uo2_21_tri} ${uo2_21} ${gap_uo2}'
  []
  [hp_inner]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '2 2 2 2 2 2'
    background_intervals = 1
    background_block_ids = '${monolith_11}'
    polygon_size = ${fparse pin_pitch / 2}
    polygon_size_style = apothem
    ring_radii = '${hp_radius} ${fparse hp_radius + gap_thickness}'
    ring_intervals = '1 1'
    ring_block_ids = '${htpipe} ${gap_htpipe}'
  []
  [hp_outer]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '2 2 2 2 2 2'
    background_intervals = 1
    background_block_ids = '${monolith_21}'
    polygon_size = ${fparse pin_pitch / 2}
    polygon_size_style = apothem
    ring_radii = '${hp_radius} ${fparse hp_radius + gap_thickness}'
    ring_intervals = '1 1'
    ring_block_ids = '${htpipe} ${gap_htpipe}'
  []
  [solid]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '2 2 2 2 2 2'
    background_intervals = 3
    background_block_ids = '${monolith_cr_tri} ${monolith_cr}'
    polygon_size = ${fparse pin_pitch / 2}
    polygon_size_style = apothem
  []
  [assembly_inner]
    type = PatternedHexMeshGenerator
    inputs = 'fuel_inner hp_inner'
    background_intervals = 1
    background_block_id = ${monolith_11}
    hexagon_size = ${fparse asy_pitch / 2}
    hexagon_size_style = apothem
    id_name = 'local_pin_id'
    pattern = '0 0 0 0 0 0;
              0 1 0 1 0 1 0;
             0 0 0 0 0 0 0 0;
            0 1 0 1 0 1 0 1 0;
           0 0 0 0 0 0 0 0 0 0;
          0 1 0 1 0 1 0 1 0 1 0;
           0 0 0 0 0 0 0 0 0 0;
            0 1 0 1 0 1 0 1 0;
             0 0 0 0 0 0 0 0;
              0 1 0 1 0 1 0;
               0 0 0 0 0 0'
  []
  [assembly_outer]
    type = PatternedHexMeshGenerator
    inputs = 'fuel_outer hp_outer'
    background_intervals = 1
    background_block_id = ${monolith_21}
    hexagon_size = ${fparse asy_pitch / 2}
    hexagon_size_style = apothem
    id_name = 'local_pin_id'
    pattern = '0 0 0 0 0 0;
              0 1 0 1 0 1 0;
             0 0 0 0 0 0 0 0;
            0 1 0 1 0 1 0 1 0;
           0 0 0 0 0 0 0 0 0 0;
          0 1 0 1 0 1 0 1 0 1 0;
           0 0 0 0 0 0 0 0 0 0;
            0 1 0 1 0 1 0 1 0;
             0 0 0 0 0 0 0 0;
              0 1 0 1 0 1 0;
               0 0 0 0 0 0'
  []
  [block]
    type = PatternedHexMeshGenerator
    inputs = 'solid'
    background_intervals = 1
    background_block_id = ${monolith_cr}
    hexagon_size = ${fparse asy_pitch / 2}
    hexagon_size_style = apothem
    pattern = '0 0 0 0 0 0;
              0 0 0 0 0 0 0;
             0 0 0 0 0 0 0 0;
            0 0 0 0 0 0 0 0 0;
           0 0 0 0 0 0 0 0 0 0;
          0 0 0 0 0 0 0 0 0 0 0;
           0 0 0 0 0 0 0 0 0 0;
            0 0 0 0 0 0 0 0 0;
             0 0 0 0 0 0 0 0;
              0 0 0 0 0 0 0;
               0 0 0 0 0 0'
  []
  [core]
    type = PatternedHexMeshGenerator
    inputs = 'assembly_outer assembly_inner block'
    id_name = 'assembly_id assembly_ring_id'
    assign_type = 'cell pattern'
    exclude_id = 'block'
    pattern = '0 0 0;
              0 1 1 0;
             0 1 2 1 0;
              0 1 1 0;
               0 0 0'
    generate_core_metadata = true
    pattern_boundary = none
  []
  [monolith_outer]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '50 50 50 50 50 50'
    background_block_ids = '${monolith_rr}'
    duct_block_ids = '${gap_reflector}'
    duct_intervals = '1'
    duct_sizes = '${fparse monolith_pitch / 2}'
    duct_sizes_style = apothem
    polygon_size = ${fparse (monolith_pitch + gap_thickness) / 2}
    polygon_size_style = apothem
    quad_center_elements = true
  []
  [monolith_gap]
    type = BlockDeletionGenerator
    input = monolith_outer
    block = '${monolith_rr}'
    new_boundary = 'INNER'
  []
  [monolith_inner]
    type = XYDelaunayGenerator
    boundary = monolith_gap
    holes = 'core'
    stitch_holes = 'true'
    refine_holes = 'false'
    verify_holes = false
    desired_area = 2
    input_boundary_names = 'INNER'
    output_boundary = 'SURFACE'
    output_subdomain_name = '${monolith_rr_tri}'
  []
  [monolith]
    type = StitchedMeshGenerator
    inputs = 'monolith_inner monolith_gap'
    stitch_boundaries_pairs = 'SURFACE INNER'
  []
  [periphery]
    type = AdvancedConcentricCircleGenerator
    ring_radii = '${fparse canister_diameter / 2}'
    ring_intervals = '1'
    num_sectors = 200
  []
  [reflector]
    type = XYDelaunayGenerator
    boundary = periphery
    holes = 'monolith'
    stitch_holes = 'true'
    refine_holes = 'false'
    verify_holes = false
    desired_area = 2
    output_subdomain_name = '${reflector_tri}'
    output_boundary = 'outer'
  []
  [extrude]
    type = AdvancedExtruderGenerator
    input = reflector
    heights = '20 30 100 30 20'
    num_layers = '2 3 10 3 2'
    direction = '0 0 1'
    bottom_boundary = '102'
    top_boundary = '103'
    subdomain_swaps = '${uo2_11} ${monolith_lr} ${uo2_11_tri} ${monolith_lr_tri} ${uo2_21} ${monolith_lr} ${uo2_21_tri} ${monolith_lr_tri}
                       ${monolith_11} ${monolith_lr} ${monolith_21} ${monolith_lr} ${monolith_cr} ${monolith_lr} ${monolith_cr_tri} ${monolith_lr_tri} ${monolith_rr_tri} ${monolith_lr_tri}
                       ${htpipe} ${monolith_lr_tri}
                       ${gap_uo2} ${monolith_lr} ${gap_htpipe} ${monolith_lr};
                       ;
                       ${uo2_11} ${uo2_12} ${uo2_11_tri} ${uo2_12_tri} ${uo2_21} ${uo2_22} ${uo2_21_tri} ${uo2_22_tri}
                       ${monolith_11} ${monolith_12} ${monolith_21} ${monolith_22};
                       ${uo2_11} ${uo2_13} ${uo2_11_tri} ${uo2_13_tri} ${uo2_21} ${uo2_23} ${uo2_21_tri} ${uo2_23_tri}
                       ${monolith_11} ${monolith_13} ${monolith_21} ${monolith_23};
                       ${uo2_11} ${monolith_ur} ${uo2_11_tri} ${monolith_ur_tri} ${uo2_21} ${monolith_ur} ${uo2_21_tri} ${monolith_ur_tri}
                       ${monolith_11} ${monolith_ur} ${monolith_21} ${monolith_ur} ${monolith_cr} ${monolith_ur} ${monolith_cr_tri} ${monolith_ur_tri} ${monolith_rr_tri} ${monolith_ur_tri}
                       ${gap_uo2} ${monolith_ur}'
  []
  [fuel_inner_sideset]
    type = SideSetsBetweenSubdomainsGenerator
    input = extrude
    primary_block = '${uo2_11} ${uo2_12} ${uo2_13} ${uo2_21} ${uo2_22} ${uo2_23}'
    paired_block = '${gap_uo2} ${gap_uo2} ${gap_uo2} ${gap_uo2} ${gap_uo2} ${gap_uo2}'
    new_boundary = 'fuel_inner'
  []
  [fuel_outer_sideset]
    type = SideSetsBetweenSubdomainsGenerator
    input = fuel_inner_sideset
    primary_block = '${monolith_11} ${monolith_12} ${monolith_13} ${monolith_21} ${monolith_22} ${monolith_23}'
    paired_block = '${gap_uo2} ${gap_uo2} ${gap_uo2} ${gap_uo2} ${gap_uo2} ${gap_uo2}'
    new_boundary = 'fuel_outer'
  []
  [htpipe_sideset]
    type = SideSetsBetweenSubdomainsGenerator
    input = fuel_outer_sideset
    primary_block = '${monolith_11} ${monolith_12} ${monolith_13} ${monolith_21} ${monolith_22} ${monolith_23} ${monolith_ur}'
    paired_block = '${gap_htpipe} ${gap_htpipe} ${gap_htpipe} ${gap_htpipe} ${gap_htpipe} ${gap_htpipe} ${gap_htpipe}'
    new_boundary = 'htpipe'
  []
  [monolith_sideset]
    type = SideSetsBetweenSubdomainsGenerator
    input = htpipe_sideset
    primary_block = '${monolith_lr_tri} ${monolith_rr_tri} ${monolith_ur_tri}'
    paired_block = '${gap_reflector} ${gap_reflector} ${gap_reflector}'
    new_boundary = 'monolith'
  []
  [reflector_sideset]
    type = SideSetsBetweenSubdomainsGenerator
    input = monolith_sideset
    primary_block = '${reflector_tri}'
    paired_block = '${gap_reflector}'
    new_boundary = 'reflector'
  []
  [assign_material]
    type = SubdomainExtraElementIDGenerator
    input = reflector_sideset
    subdomains = '${uo2_11} ${uo2_12} ${uo2_13} ${uo2_21} ${uo2_22} ${uo2_23}
                  ${uo2_11_tri} ${uo2_12_tri} ${uo2_13_tri} ${uo2_21_tri} ${uo2_22_tri} ${uo2_23_tri}
                  ${monolith_11} ${monolith_12} ${monolith_13} ${monolith_21} ${monolith_22} ${monolith_23}
                  ${monolith_lr} ${monolith_ur} ${monolith_cr}
                  ${monolith_lr_tri} ${monolith_ur_tri} ${monolith_rr_tri} ${monolith_cr_tri}
                  ${htpipe}
                  ${reflector_tri}
                  ${gap_uo2} ${gap_htpipe} ${gap_reflector}'
    extra_element_id_names = 'material_id'
    extra_element_ids = '${uo2_11} ${uo2_12} ${uo2_13} ${uo2_21} ${uo2_22} ${uo2_23}
                         ${uo2_11} ${uo2_12} ${uo2_13} ${uo2_21} ${uo2_22} ${uo2_23}
                         ${monolith_11} ${monolith_12} ${monolith_13} ${monolith_21} ${monolith_22} ${monolith_23}
                         ${monolith_lr} ${monolith_ur} ${monolith_cr}
                         ${monolith_lr} ${monolith_ur} ${monolith_rr} ${monolith_cr}
                         ${htpipe}
                         ${reflector}
                         ${gap} ${gap} ${gap}'
  []
  [finalize_block]
    type = RenameBlockGenerator
    input = assign_material
    old_block = '${uo2_11} ${uo2_12} ${uo2_13} ${uo2_21} ${uo2_22} ${uo2_23}
                 ${uo2_11_tri} ${uo2_12_tri} ${uo2_13_tri} ${uo2_21_tri} ${uo2_22_tri} ${uo2_23_tri}
                 ${monolith_11} ${monolith_12} ${monolith_13} ${monolith_21} ${monolith_22} ${monolith_23}
                 ${monolith_lr} ${monolith_ur} ${monolith_cr}
                 ${monolith_lr_tri} ${monolith_ur_tri} ${monolith_rr_tri} ${monolith_cr_tri}
                 ${htpipe}
                 ${reflector_tri}
                 ${gap_uo2} ${gap_htpipe} ${gap_reflector}'
    new_block = 'FUEL_11 FUEL_12 FUEL_13 FUEL_21 FUEL_22 FUEL_23
                 FUEL_TRI_11 FUEL_TRI_12 FUEL_TRI_13 FUEL_TRI_21 FUEL_TRI_22 FUEL_TRI_23
                 MONOLITH MONOLITH MONOLITH MONOLITH MONOLITH MONOLITH
                 REFLECTOR REFLECTOR MONOLITH
                 REFLECTOR_TRI REFLECTOR_TRI MONOLITH_TRI MONOLITH_TRI
                 HTPIPE
                 REFLECTOR_TRI
                 GAP GAP GAP'
  []
  [boundary]
    type = RenameBoundaryGenerator
    input = finalize_block
    old_boundary = '102 103'
    new_boundary = 'bottom top'
  []
  [scale]
    type = TransformGenerator
    input = boundary
    transform = SCALE
    vector_value = '0.01 0.01 0.01'
  []
  [delete]
    type = BlockDeletionGenerator
    input = scale
    block = 'HTPIPE'
  []
[]