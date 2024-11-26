[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 1
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
  [diffusion]
    type = LinearFVAnisotropicDiffusion
    variable = u
    diffusion_tensor = diffusivity_tensor
    use_nonorthogonal_correction = false
  []
  [source]
    type = LinearFVSource
    variable = u
    source_density = source_func
  []
[]

[LinearFVBCs]
  [dir]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = "left right top bottom"
    functor = analytic_solution
  []
[]

[FunctorMaterials]
  [diff_tensor]
    type = GenericVectorFunctorMaterial
    prop_names = diffusivity_tensor
    prop_values = 'coeff_func_x coeff_func_y 0.0'
  []
[]

[Functions]
  [coeff_func_x]
    type = ParsedFunction
    expression = '1+0.5*x*y'
  []
  [coeff_func_y]
    type = ParsedFunction
    expression = '1+x*y'
  []
  [source_func]
    type = ParsedFunction
    expression = '(1.5-y*y)*(2+2*x*y)+(1.5-x*x)*(2+4*x*y)'
  []
  [analytic_solution]
    type = ParsedFunction
    expression = '(1.5-x*x)*(1.5-y*y)'
  []
[]

[Postprocessors]
  [h]
    type = AverageElementSize
    execute_on = FINAL
  []
  [error]
    type = ElementL2FunctorError
    approximate = u
    exact = analytic_solution
    execute_on = FINAL
  []
[]

[Convergence]
  [linear]
    type = IterationCountConvergence
    max_iterations = 1
    converge_at_max_iterations = true
  []
[]

[Executioner]
  type = Steady
  system_names = u_sys
  l_abs_tol = 1e-10
  multi_system_fixed_point=true
  multi_system_fixed_point_convergence=linear
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  [csv]
    type = CSV
    execute_on = FINAL
  []
[]
