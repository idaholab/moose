[Mesh]
    [myCCMG]
        type = ConcentricCircleMeshGenerator
        num_sectors = 4
        radii = '0.94 0.96 0.98 1.0'
        rings = '1 1 1 1'
        has_outer_square = false
        preserve_volumes = false
        smoothing_max_it = 0

    []

    [myDualGen]
        type = DualMeshGenerator
        input = myCCMG
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