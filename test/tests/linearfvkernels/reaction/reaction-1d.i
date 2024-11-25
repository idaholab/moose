[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
  []
[]

[Problem]
  linear_sys_names = 'u_sys'
[]

[Variables]
  [u]
    type = MooseLinearVariableFVReal
    solver_sys = 'u_sys'
    initial_condition = 1.0
  []
[]

[LinearFVKernels]
  [reaction]
    type = LinearFVReaction
    variable = u
    coeff = coeff_func
  []
  [source]
    type = LinearFVSource
    variable = u
    source_density = source_func
  []
[]

[Functions]
  [coeff_func]
    type = ParsedFunction
    expression = '1+sin(x)'
  []
  [source_func]
    type = ParsedFunction
    expression = '(1+sin(x))*(1+cos(x))'
  []
  [analytic_solution]
    type = ParsedFunction
    expression = '1+cos(x)'
  []
[]

[Postprocessors]
  [l2error]
    type = ElementL2FunctorError
    approximate = u
    exact = analytic_solution
    execute_on = TIMESTEP_END
  []
[]

[Executioner]
  type = Steady
  system_names = u_sys
  l_abs_tol = 1e-10
[]

[Outputs]
  [exodus]
    type = Exodus
    execute_on = TIMESTEP_END
  []
[]
