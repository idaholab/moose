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
  [diffusion]
    type = LinearFVDiffusionKernel
    variable = u
    diffusion_coeff = coeff_func
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
    functor = 1.0
  []
  [right_dir]
    type = LinearFVFunctorDirichletBC
    variable = u
    boundary = "right"
    functor = 2.0
  []
[]

[Functions]
  [coeff_func]
    type = ParsedFunction
    expression = '0.5*x'
  []
  [source_func]
    type = ParsedFunction
    expression = '2*x'
  []
  [analytic_solution]
    type = ParsedFunction
    expression = '1-x*x'
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
