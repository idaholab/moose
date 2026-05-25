[Mesh]
    [myCCMG]
        type = ConcentricCircleMeshGenerator
        num_sectors = 4
        radii = '2'
        rings = '2 1'
        pitch = 5
        has_outer_square = true
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
    
    [myDualGen]
        type = DualMeshGenerator
        input = cut_center
    []

    [SdmPerElemGen]
        type = SubdomainPerElementGenerator
        input = myDualGen
    []

    [convert]
        type = ElementsToSimplicesConverter
        input = 'SdmPerElemGen'
    []


[]