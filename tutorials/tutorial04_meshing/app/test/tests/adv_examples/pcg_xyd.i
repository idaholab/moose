[Mesh]
    # make a unit hexagonal pin
    [hex_1]
      type = PolygonConcentricCircleMeshGenerator
      num_sides = 6
      num_sectors_per_side = '2 2 2 2 2 2'
      background_intervals = 2
      ring_radii = 4.0
      ring_intervals = 2
      ring_block_ids = '10 15'
      ring_block_names = 'pin_tri pin_quad'
      background_block_ids = 20
      background_block_names = pin_background
      polygon_size = 5.0
      preserve_volumes = on
    []
    # stitch the unit pins into a hexagonal pattern
    [pattern]
      type = PatternedHexMeshGenerator
      inputs = 'hex_1'
      hexagon_size=25
      background_block_id = 30
      background_block_name = assembly_background
      pattern = '0 0 0;
                0 0 0 0;
               0 0 0 0 0;
                0 0 0 0;
                 0 0 0'
    []
    # Add a layer of circular peripheral region
    [tmg1]
        type = PeripheralTriangleMeshGenerator
        input = pattern
        peripheral_ring_radius = 35
        peripheral_ring_num_segments = 50
        peripheral_ring_block_name = peripheral_1
        desired_area = 8
    []
    # Add another layer of circular peripheral region
    [tmg2]
        type = PeripheralTriangleMeshGenerator
        input = tmg1
        peripheral_ring_radius = 38
        peripheral_ring_num_segments = 80
        peripheral_ring_block_name = peripheral_2
        desired_area = 6
    []
    # Define the complex boundary shape using ParsedCurveMG
    [pcg1]
        type = ParsedCurveGenerator
        x_formula = 't1:=t;
                     t2:=t-1;
                     t3:=t-2;
                     t4:=t-3;
                     x1:=r*cos(t1*(th1-th0)+th0);
                     x2_0:=r*cos(th1);
                     x2_1:=x2_0-Lx;
                     x2:=x2_0+(x2_1-x2_0)*t2;
                     rs:=abs(r*sin(th1));
                     rth0:=1.5*pi;
                     rth1:=0.5*pi;
                     x3:=x2_1+rs*cos(rth0+(rth1-rth0)*t3);
                     x4_1:=r*cos(th1);
                     x4_0:=x4_1-Lx;
                     x4:=x4_0+(x4_1-x4_0)*t4;
                     if(t<1,x1,if(t<2,x2,if(t<3,x3,x4)))'
        y_formula = 't1:=t;
                     t2:=t-1;
                     t3:=t-2;
                     t4:=t-3;
                     y1:=r*sin(t1*(th1-th0)+th0);
                     y2:=r*sin(th1);
                     rs:=abs(r*sin(th1));
                     rth0:=1.5*pi;
                     rth1:=0.5*pi;
                     y3:=rs*sin(rth0+(rth1-rth0)*t3);
                     y4:=r*sin(th0);
                     if(t<1,y1,if(t<2,y2,if(t<3,y3,y4)))'
        section_bounding_t_values  = '0 1 2 3 4'
        constant_names =       'pi           r    th0               th1                    Lx'
        constant_expressions = '${fparse pi} 100.0 ${fparse pi/9.0} ${fparse pi/9.0*17.0} 10.0'
        nums_segments = '50 3 15 3'
        is_closed_loop = true
    []
    # fill the curve defined above and include the internal patterned pin regions
    [xydg1]
        type = XYDelaunayGenerator
        boundary = 'pcg1'
        holes = 'tmg2'
        add_nodes_per_boundary_segment = 0
        refine_boundary = false
        desired_area = 30
        output_subdomain_name = xy_layer_1
        stitch_holes = 'true'
        refine_holes = 'false'
    []
    # Define the control drum pin structure
    # As we currently do not have a pure ring mesher
    # We will use PCCMG and then delete the hexagonal outermost layer to get the rings
    [cd]
      type = PolygonConcentricCircleMeshGenerator
      num_sides = 6
      num_sectors_per_side = '6 6 6 6 6 6'
      background_intervals = 2
      ring_radii = '27 30 32'
      ring_intervals = '2 2 2'
      ring_block_ids = '110 115 120 125'
      ring_block_names = 'cd_1_tri cd_1_quad cd_2 cd_3'
      background_block_ids = 130
      background_block_names = cd_background
      polygon_size = 40
      preserve_volumes = on
    []
    # Define the absorber region of the control drum
    [cd_azi_define]
        type = AzimuthalBlockSplitGenerator
        input = cd
        start_angle = 300
        angle_range = 120
        old_blocks = '120'
        new_block_ids = '121'
        new_block_names = 'absorber'
        preserve_volumes = true
    []
    # Delete the hexagonal outmost layer
    [cd_bd]
        type = BlockDeletionGenerator
        input = cd_azi_define
        block = cd_background
    []
    # Translate the control drum to desired position
    [cd_translate]
        type = TransformGenerator
        input = cd_bd
        transform = translate
        vector_value = '${fparse 100.0*cos(pi/9.0)-10.0} 0 0'
    []
    # Define the small pin near the external boundary
    # We are using the similar PCCMG+deletion approach
    [outer_pin]
      type = PolygonConcentricCircleMeshGenerator
      num_sides = 6
      num_sectors_per_side = '2 2 2 2 2 2'
      background_intervals = 2
      ring_radii = '10 12'
      ring_intervals = '2 2'
      ring_block_ids = '210 215 220'
      ring_block_names = 'op_1_tri op_1_quad op_2'
      background_block_ids = 230
      background_block_names = op_background
      polygon_size = 15
      preserve_volumes = on
    []
    # delete the outermost layer to get a circular pin mesh
    [op_bd]
        type = BlockDeletionGenerator
        input = outer_pin
        block = op_background
    []
    # translate to the desired position
    [op_translate]
        type = TransformGenerator
        input = op_bd
        transform = translate
        vector_value = '${fparse 120/sqrt(2)} ${fparse 120/sqrt(2)} 0'
    []
    # Define the circular curve boundary
    [pcg2]
        type = ParsedCurveGenerator
        x_formula = 'r*cos(t)'
        y_formula = 'r*sin(t)'
        section_bounding_t_values = '0 ${fparse 2*pi}'
        constant_names =       'pi           r'
        constant_expressions = '${fparse pi} 140.0'
        nums_segments = '100'
        is_closed_loop = true
    []
    # fill the curve defined above including the internal structure, control drum and outer pin
    [xydg2]
        type = XYDelaunayGenerator
        boundary = 'pcg2'
        holes = 'xydg1 cd_translate op_translate'
        add_nodes_per_boundary_segment = 0
        refine_boundary = false
        desired_area = 30
        output_subdomain_name = xy_layer_2
        stitch_holes = 'true true true'
        refine_holes = 'false false false'
    []
    # add another layer of circular peripheral region
    [tmg3]
        type = PeripheralTriangleMeshGenerator
        input = xydg2
        peripheral_ring_radius = 150
        peripheral_ring_num_segments = 100
        peripheral_ring_block_name = peripheral_3
        desired_area = 20
    []
[]
