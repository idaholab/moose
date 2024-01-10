[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 2
  []
[]

[Problem]
  linear_sys_names = 'u_sys'
[]

[Variables]
  [u]
    type = MooseLinearVariableFVReal
    linear_sys = 'u_sys'
    initial_condition = 1.0
  []
[]

[LinearFVKernels]
  [reaction]
    type = LinearFVReactionKernel
    variable = u
    coeff = 5.0
  []
  [source]
    type = LinearFVSourceKernel
    variable = u
    source_density = source_func
  []
[]

[Functions]
  [coeff_func]
    type = ParsedFunction
    expression = '1+x'
  []
  [source_func]
    type = ParsedFunction
    expression = '1-x*x'
  []
  [analytic_solution]
    type = ParsedFunction
    expression = '1-x'
  []
[]

[Postprocessors]
  [l2error]
    type = ElementL2FunctorError
    approximate = u
    exact = analytic_solution
    execute_on = FINAL
  []
[]

[Executioner]
  type = LinearPicardSteady
  linear_systems_to_solve = u_sys
[]

[Outputs]
  [csv]
    type = CSV
    execute_on = FINAL
  []
[]
