diffusion_coef = 1.0
opacity = 1.0
temperature_radiation = 100.0
G_bc = 1.0
sigma = 5.670374419e-8

[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 50
  []
[]

[Variables]
  [G]
    type = MooseVariableFVReal
    initial_condition = 1
  []
[]

[FVKernels]
  [G_diffusion]
    type = FVDiffusion
    variable = G
    coeff = ${diffusion_coef}
  []
  [source_and_sink]
    type = FVThermalRadiationSourceSink
    variable = G
    temperature_radiation = ${temperature_radiation}
    opacity = ${opacity}
  []
[]

[FVBCs]
  [right_bc]
    type = FVDirichletBC
    boundary = 'right'
    variable = G
    value = ${G_bc}
  []
[]

[Functions]
  [analytical_sol]
    type = ParsedFunction
    symbol_names = 'a'
    symbol_values = '${fparse sqrt(opacity / diffusion_coef)}'
    expression = '${G_bc} * cosh(a*x) / cosh(a) + ${sigma} * ${temperature_radiation}^4 * (1.0 - cosh(a*x) / cosh(a))'
  []
[]

[Postprocessors]
  [value_solution]
    type = ElementIntegralFunctorPostprocessor
    functor = G
  []
  [value_analytic]
    type = ElementIntegralFunctorPostprocessor
    functor = analytical_sol
  []
  [relative_difference]
    type = RelativeDifferencePostprocessor
    value1 = value_solution
    value2 = value_analytic
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  nl_abs_tol = 1e-12
[]

[Outputs]
  exodus = false
  csv = true
[]
