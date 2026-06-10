[Mesh]
    [myCCMG]
        type = ConcentricCircleMeshGenerator

        num_sectors = 4
        radii = '0.94 0.96 0.98 1.0'
        rings = '5 1 1 1'
        has_outer_square = false
        preserve_volumes = false
    []

    [SdmPerElemGen]
        type = SubdomainPerElementGenerator
        input = myCCMG
    []

    [convert]
        type = ElementsToSimplicesConverter
        input = 'SdmPerElemGen'
    []

[]