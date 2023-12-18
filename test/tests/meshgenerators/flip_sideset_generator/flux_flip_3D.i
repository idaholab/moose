[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 3
    ny = 3
    nz = 3
    xmax = 3
    ymax = 3
    zmax = 3
  []
  [subdomains]
    type = ParsedSubdomainMeshGenerator
    input = gmg
    combinatorial_geometry = 'x < 1 & y > 1 & y < 2'
    block_id = 1
  []
  [sideset]
    type = ParsedGenerateSideset
    input = subdomains
    combinatorial_geometry = 'z < 1'
    included_subdomains = '1'
    normal = '1 0 0'
    new_sideset_name = interior
  []
  [flip]
    type = FlipSidesetGenerator
    input = sideset
    boundary = interior
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
        expression = x+y+z
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
        boundary = interior
        diffusivity = 1
    []
    [area]
        type = AreaPostprocessor
        boundary = interior
    []
[]

[Executioner]
    type = Steady
[]

[Outputs]
    csv = true
[]
