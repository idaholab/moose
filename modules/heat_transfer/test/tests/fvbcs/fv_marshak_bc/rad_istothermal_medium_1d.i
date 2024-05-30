# The test gurantees that Marshak BCs yield the expected constant 1D solution

diffusion_coef = 1e-12
opacity = 1.0
temperature_radiation = 100.0
wall_temperature = ${fparse temperature_radiation / (4^(1/4))}
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
    type = FVMarshakRadiativeBC
    boundary = 'right'
    variable = G
    temperature_radiation = ${wall_temperature}
    coeff_diffusion = ${diffusion_coef}
    boundary_emissivity = 1.0
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
  [mean_value]
    type = ElementIntegralFunctorPostprocessor
    functor = G
  []
  [max_value]
    type = ElementExtremeFunctorValue
    functor = G
  []
  [relative_difference]
    type = ParsedPostprocessor
    pp_names = 'mean_value max_value'
    expression = '(max_value / mean_value - 1.0) / mean_value'
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
