[Mesh]
    [myCCMG]
        type = ConcentricCircleMeshGenerator
        num_sectors = 4
        radii = '2 4 7'
        rings = '3 3 4'
        has_outer_square = false
        preserve_volumes = false
    []

    [mark_center]
        type = ParsedSubdomainMeshGenerator
        input = myCCMG
        combinatorial_geometry = 'x*x + y*y < 4.0'
        block_id = 10
    []

    [cut_center]
        type = BlockDeletionGenerator
        input = mark_center
        block = 10
    []

    [extrude]
        type = AdvancedExtruderGenerator
        input = cut_center
        direction = '0 0 1'
        heights = '5'
        num_layers = '3'
    []

    [myDualGen]
        type = DualMeshGenerator
        input = extrude
        dual_mesh_type = barycentric
        concave_treatment = 'split polycut netgen'
    []

    [convert]
        type = ElementsToSimplicesConverter
        input = myDualGen
    []
[]
