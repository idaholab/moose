[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
    xmax = 5
    ymax = 5
  []
  [modify_subdomain]
    type = ParsedSubdomainMeshGenerator
    input = gmg
    block_id = 1
    combinatorial_geometry = 'x > 2'
  []
[]

[Materials]
  [generic_mat_block0]
    type = GenericFunctionMaterial
    block = 0
    prop_names = 'mat'
    prop_values = 'parsed_block0'
  []
  [generic_mat_block1]
    type = GenericFunctionMaterial
    block = 1
    prop_names = 'mat'
    prop_values = 'parsed_block1'
  []
[]

[Functions]
  [parsed_block0]
    type = ParsedFunction
    expression = 'x + 2 * y'
  []
  [parsed_block1] # continuous at the interface
    type = ParsedFunction
    expression = '2 * x + 2 * y - 2'
  []
[]

[UserObjects]
  [study]
    type = RepeatableRayStudy
    names = 'diag
             top_across
             bottom_across
             partial'
    start_points = '0 0 0
                    0 5 0
                    0 0 0
                    0.5 0.5 0'
    end_points = '5 5 0
                  5 5 0
                  5 0 0
                  4.5 0.5 0'
  []
[]

[RayKernels]
  [material_integral]
    type = MaterialIntegralRayKernel
    study = study
    mat_prop = mat
  []
[]

[Postprocessors]
  [diag_value]
    type = RayIntegralValue
    ray_kernel = material_integral
    ray = diag
  []
  [top_across_value]
    type = RayIntegralValue
    ray_kernel = material_integral
    ray = top_across
  []
  [bottom_across_value]
    type = RayIntegralValue
    ray_kernel = material_integral
    ray = bottom_across
  []
  [partial_value]
    type = RayIntegralValue
    ray_kernel = material_integral
    ray = partial
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = false
  csv = true
[]
