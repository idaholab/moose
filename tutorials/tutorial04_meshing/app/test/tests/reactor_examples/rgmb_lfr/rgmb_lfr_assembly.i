[Mesh]
  [rmp]
    type = ReactorMeshParams
    dim = 3
    geom = "Hex"
    assembly_pitch = 16.4165
    axial_regions = '10.07 30.79 6.56 85.85 1.52 106.07 1.51 12.13 5.05 93.87'
    axial_mesh_intervals = '1 3 1 9 1 20 1 2 1 9'
    top_boundary_id = 201
    bottom_boundary_id = 202
    radial_boundary_id = 200
  []
  [pin1]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 1
    pitch = 1.3425
    num_sectors = 2
    mesh_intervals = '1 3 1 1 1'
    ring_radii = '0.2020 0.4319 0.4495 0.5404'
    region_ids='1 1 1 1 1;
                2 2 2 2 2;
                3 3 3 3 3;
                4 4 4 5 6;
                8 8 8 9 10;
                20 19 20 13 21;
                24 24 24 25 26;
                28 28 28 29 30;
                32 32 32 32 32;
                33 33 33 33 33'
    quad_center_elements = false
  []
  [pin2]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 2
    pitch = 1.3425
    num_sectors = 2
    mesh_intervals = '1 3 1 1 1'
    ring_radii = '0.2020 0.4319 0.4495 0.5404'
    region_ids='1 1 1 1 1;
                2 2 2 2 2;
                3 3 3 3 3;
                4 4 4 5 6;
                8 8 8 9 10;
                20 18 20 13 21;
                24 24 24 25 26;
                28 28 28 29 30;
                32 32 32 32 32;
                33 33 33 33 33'
    quad_center_elements = false
  []
  [pin3]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 3
    pitch = 1.3425
    num_sectors = 2
    mesh_intervals = '1 3 1 1 1'
    ring_radii = '0.2020 0.4319 0.4495 0.5404'
    region_ids='1 1 1 1 1;
                2 2 2 2 2;
                3 3 3 3 3;
                4 4 4 5 6;
                8 8 8 9 10;
                20 17 20 13 21;
                24 24 24 25 26;
                28 28 28 29 30;
                32 32 32 32 32;
                33 33 33 33 33'
    quad_center_elements = false
  []
  [pin4]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 4
    pitch = 1.3425
    num_sectors = 2
    mesh_intervals = '1 3 1 1 1'
    ring_radii = '0.2020 0.4319 0.4495 0.5404'
    region_ids='1 1 1 1 1;
                2 2 2 2 2;
                3 3 3 3 3;
                4 4 4 5 6;
                8 8 8 9 10;
                20 16 20 13 21;
                24 24 24 25 26;
                28 28 28 29 30;
                32 32 32 32 32;
                33 33 33 33 33'
    quad_center_elements = false
  []
  [pin5]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 5
    pitch = 1.3425
    num_sectors = 2
    mesh_intervals = '1 3 1 1 1'
    ring_radii = '0.2020 0.4319 0.4495 0.5404'
    region_ids='1 1 1 1 1;
                2 2 2 2 2;
                3 3 3 3 3;
                4 4 4 5 6;
                8 8 8 9 10;
                20 15 20 13 21;
                24 24 24 25 26;
                28 28 28 29 30;
                32 32 32 32 32;
                33 33 33 33 33'
    quad_center_elements = false
  []
  [pin6]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 6
    pitch = 1.3425
    num_sectors = 2
    mesh_intervals = '1 3 1 1 1'
    ring_radii = '0.2020 0.4319 0.4495 0.5404'
    region_ids='1 1 1 1 1;
                2 2 2 2 2;
                3 3 3 3 3;
                4 4 4 5 6;
                8 8 8 9 10;
                20 14 20 13 21;
                24 24 24 25 26;
                28 28 28 29 30;
                32 32 32 32 32;
                33 33 33 33 33'
    quad_center_elements = false
  []
  [pin7]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 7
    pitch = 1.3425
    num_sectors = 2
    mesh_intervals = '1 3 1 1 1'
    ring_radii = '0.2020 0.4319 0.4495 0.5404'
    region_ids='1 1 1 1 1;
                2 2 2 2 2;
                3 3 3 3 3;
                4 4 4 5 6;
                8 8 8 9 10;
                20 12 20 13 21;
                24 24 24 25 26;
                28 28 28 29 30;
                32 32 32 32 32;
                33 33 33 33 33'
    quad_center_elements = false
  []
  [assembly]
    type = AssemblyMeshGenerator
    inputs = 'pin1 pin2 pin3 pin4 pin5 pin6 pin7'
    pattern =  ' 0 0 0 0 0 0 0;
                0 1 1 1 1 1 1 0;
               0 1 2 2 2 2 2 1 0;
              0 1 2 3 3 3 3 2 1 0;
             0 1 2 3 4 4 4 3 2 1 0;
            0 1 2 3 4 5 5 4 3 2 1 0;
           0 1 2 3 4 5 6 5 4 3 2 1 0;
            0 1 2 3 4 5 5 4 3 2 1 0;
             0 1 2 3 4 4 4 3 2 1 0;
              0 1 2 3 3 3 3 2 1 0;
               0 1 2 2 2 2 2 1 0;
                0 1 1 1 1 1 1 0;
                 0 0 0 0 0 0 0'
    extrude = true
    assembly_type = 1
    background_region_id = '1 2 3 6 10 21 26 30 32 33'
    background_intervals = '1'
    duct_halfpitch = '7.6712 8.0245'
    duct_intervals = '1 1'
    duct_region_ids = '1 1; 2 2; 3 3; 7 6; 11 10;
                       22 23; 27 26; 31 30; 32 32; 33 33'
  []
  [lfr_assy]
    type = ExtraElementIDCopyGenerator
    input = assembly
    source_extra_element_id = region_id
    target_extra_element_ids = 'material_id'
  []
[]
