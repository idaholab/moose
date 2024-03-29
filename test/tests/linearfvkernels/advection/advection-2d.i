[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny= 1
    ymax = 0.5
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
  [advection]
    type = LinearFVAdvection
    variable = u
    velocity = "0.5 0 0"
    advected_interp_method = upwind
  []
  [source]
    type = LinearFVSource
    variable = u
    source_density = source_func
  []
[]

[LinearFVBCs]
  [inflow]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = "left top bottom"
    functor = analytic_solution
  []
  [outflow]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = u
    boundary = "right"
    use_two_term_expansion = false
  []
[]

[Functions]
  [source_func]
    type = ParsedFunction
    expression = '0.5*pi*sin(2*y*pi)*cos(x*pi)'
  []
  [analytic_solution]
    type = ParsedFunction
    expression = 'sin(x*pi)*sin(2*y*pi) + 1.5'
  []
[]

[Postprocessors]
  [error]
    type = ElementL2FunctorError
    approximate = u
    exact = analytic_solution
    execute_on = FINAL
  []
  [h]
    type = AverageElementSize
    execute_on = FINAL
  []
[]

[Executioner]
  type = LinearPicardSteady
  linear_systems_to_solve = u_sys
  number_of_iterations = 1
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'lu NONZERO               1e-10'
[]

[Outputs]
  [csv]
    type = CSV
    execute_on = FINAL
  []
[]
