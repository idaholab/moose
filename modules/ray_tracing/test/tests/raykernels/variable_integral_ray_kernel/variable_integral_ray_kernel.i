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

[AuxVariables/aux]
  order = CONSTANT
  family = MONOMIAL
  [InitialCondition]
    type = FunctionIC
    variable = u_ag
    function = 'x + y + cos(x)'
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
  [variable_integral]
    type = VariableIntegralRayKernel
    study = study
    variable = u
  []
  [aux_variable_integral]
    type = VariableIntegralRayKernel
    study = study
    variable = aux
  []
[]

[Postprocessors]
  [diag_value]
    type = RayIntegralValue
    ray_kernel = variable_integral
    ray = diag
  []
  [top_across_value]
    type = RayIntegralValue
    ray_kernel = variable_integral
    ray = top_across
  []
  [bottom_across_value]
    type = RayIntegralValue
    ray_kernel = variable_integral
    ray = bottom_across
  []
  [partial_value]
    type = RayIntegralValue
    ray_kernel = variable_integral
    ray = partial
  []

  [aux_diag_value]
    type = RayIntegralValue
    ray_kernel = aux_variable_integral
    ray = diag
  []
  [aux_top_across_value]
    type = RayIntegralValue
    ray_kernel = aux_variable_integral
    ray = top_across
  []
  [aux_bottom_across_value]
    type = RayIntegralValue
    ray_kernel = aux_variable_integral
    ray = bottom_across
  []
  [aux_partial_value]
    type = RayIntegralValue
    ray_kernel = aux_variable_integral
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
