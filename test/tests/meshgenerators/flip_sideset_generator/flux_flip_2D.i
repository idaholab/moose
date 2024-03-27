[Mesh]
    [gmg]
        type = GeneratedMeshGenerator
        dim = 2
        nx = 3
        ny = 3
        xmax = 3
        ymax = 3
    []
    [s1]
        type = ParsedGenerateSideset
        input = gmg
        combinatorial_geometry = 'x > 0.9 & x < 1.1 & y > -0.1 & y < 1.1'
        normal = '1 0 0'
        new_sideset_name = s1
    []
    [s2]
        type = ParsedGenerateSideset
        input = s1
        combinatorial_geometry = 'x > 0.9 & x < 2.1 & y > 0.9 & y < 1.1'
        normal = '0 1 0'
        new_sideset_name = s2
    []
    [s3]
        type = ParsedGenerateSideset
        input = s2
        combinatorial_geometry = 'x > 1.9 & x < 2.1 & y > 0.9 & y < 2.1'
        normal = '1 0 0'
        new_sideset_name = s3
    []
    [s4]
        type = ParsedGenerateSideset
        input = s3
        combinatorial_geometry = 'x > 1.9 & x < 3.1 & y > 1.9 & y < 2.1'
        normal = '0 1 0'
        new_sideset_name = s4
    []
    [sideset]
        type = SideSetsFromBoundingBoxGenerator
        input = s4
        bottom_left = '0 0 0'
        top_right = '3 3 3'
        included_boundaries = 's1 s2 s3 s4'
        boundary_new = 's_combined'
    []
    [flip]
        type = FlipSidesetGenerator
        input = sideset
        boundary = s_combined
    []
[]

[AuxVariables]
    [u]
    []
[]

[AuxKernels]
    [diffusion]
        type = FunctionAux
        variable = u
        function = func
    []
[]

[Functions]
    [func]
        type = ParsedFunction
        expression = x+y
    []
[]

[Problem]
    type = FEProblem
    solve = false
[]

[Postprocessors]
    [flux]
        type = SideDiffusiveFluxIntegral
        variable = u
        boundary = s_combined
        diffusivity = 1
    []
    [area]
        type = AreaPostprocessor
        boundary = s_combined
    []
[]

[Executioner]
    type = Steady
[]

[Outputs]
    csv = true
[]
