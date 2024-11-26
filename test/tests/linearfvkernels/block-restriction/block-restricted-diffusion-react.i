source=1
diff_coeff=2
reac_coeff=3

[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 1
    dx = '0.5 0.5'
    ix = '20 20'
    subdomain_id = '1 2'
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
    type = LinearFVDiffusion
    variable = u
    diffusion_coeff = ${diff_coeff}
    use_nonorthogonal_correction = false
    block = 1
  []
  [reaction]
    type = LinearFVReaction
    variable = u
    coeff = ${reac_coeff}
    block = 2
  []
  [source]
    type = LinearFVSource
    variable = u
    source_density = ${source}
  []
[]

[LinearFVBCs]
  [dir]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = "left"
    functor = 0
  []
[]

[Functions]
  [analytic_solution]
    type = ParsedFunction
    expression = 'if(x<0.5, -x*x*S/2/D+(S/C+0.5*0.5/2/D*S)/0.5*x, S/C)'
    symbol_names = 'S D C'
    symbol_values = '${source} ${diff_coeff} ${reac_coeff}'
  []
[]

[Postprocessors]
  [h]
    type = AverageElementSize
    execute_on = TIMESTEP_END
    block = 2
  []
  [error]
    type = ElementL2FunctorError
    approximate = u
    exact = analytic_solution
    execute_on = TIMESTEP_END
    block = 2
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
    execute_on = TIMESTEP_END
  []
[]
