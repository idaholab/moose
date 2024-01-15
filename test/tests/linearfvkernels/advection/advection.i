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
  [advection]
    type = LinearFVAdvectionKernel
    variable = u
    velocity = "0.5 0 0"
    advected_interp_method = average
  []
  [source]
    type = LinearFVSourceKernel
    variable = u
    source_density = source_func
  []
[]

[LinearFVBCs]
  [inflow]
    type = LinearFVFunctorDirichletBC
    variable = u
    boundary = "left"
    functor = analytic_solution
  []
  [outflow]
    type = LinearFVOutflowBC
    variable = u
    boundary = "right"
    velocity = "0.5 0 0"
    use_two_term_expansion = true
  []
[]

[Functions]
  [source_func]
    type = ParsedFunction
    expression = '0.5*x'
  []
  [analytic_solution]
    type = ParsedFunction
    expression = '0.5+0.5*x*x'
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
  number_of_iterations = 1
[]

[Outputs]
  [csv]
    type = CSV
    execute_on = FINAL
  []
  [exo]
    type = Exodus
    execute_on = FINAL
  []
[]
