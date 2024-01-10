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
  [left_dir]
    type = LinearFVFunctorDirichletBC
    variable = u
    boundary = "left"
    functor = 0.5
  []
  [right_dir]
    type = LinearFVFunctorDirichletBC
    variable = u
    boundary = "right"
    functor = 1.0
  []
[]

[Functions]
  [source_func]
    type = ParsedFunction
    expression = 'x'
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
[]

[Outputs]
  [csv]
    type = CSV
    execute_on = FINAL
  []
[]
