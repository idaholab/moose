[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
    xmax = 5
    ymax = 5
  []
[]

[Variables/u]
  [InitialCondition]
    type = FunctionIC
    variable = u
    function = '(x < 2) * (x + 2 * y) + (x >= 2) * (2 * x + 2 * y - 2)'
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
    always_cache_traces = true
    data_on_cache_traces = true
    aux_data_on_cache_traces = true
    ray_aux_data_names = 'test_aux'
    initial_ray_aux_data = '1; 2; 3; 4'
  []
[]

[RayKernels]
  [variable_integral]
    type = VariableIntegralRayKernel
    study = study
    variable = u
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
  [rays]
    type = RayTracingExodus
    study = study
    output_data = true
    output_aux_data = true
    execute_on = final
  []
  [rays_nodal]
    type = RayTracingExodus
    study = study
    output_data_nodal = true
    execute_on = final
  []
[]
