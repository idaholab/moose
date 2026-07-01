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
        dual_mesh_type = voronoi
    []

    [myDiagnosticGenerator]
        type = MeshDiagnosticsGenerator
        input = myDualGen
        examine_element_overlap = ERROR
        examine_element_volumes = ERROR
        minimum_element_volumes = 0
    []

    [convert]
        type = ElementsToSimplicesConverter
        input = myDiagnosticGenerator
    []
[]
