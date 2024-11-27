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

[AuxVariables]
  [v_volume]
    type = MooseLinearVariableFVReal
    initial_condition = 50
  []
  [v_functor]
    type = MooseLinearVariableFVReal
    initial_condition = 25
  []
  [v_parsed]
    type = MooseLinearVariableFVReal
    initial_condition = 12.5
  []
[]

[AuxKernels]
  [volume]
    type = VolumeAux
    variable = v_volume
  []
  [functor]
    type = FunctorAux
    variable = v_functor
    functor = u
  []
  [parsed]
    type = ParsedAux
    variable = v_parsed
    coupled_variables = 'v_volume v_functor'
    expression = '0.5*v_volume+0.5*v_functor'
  []
[]

[LinearFVKernels]
  [diffusion]
    type = LinearFVDiffusion
    variable = u
    diffusion_coeff = coeff_func
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
    boundary = "left right"
    functor = analytic_solution
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

[Executioner]
  type = Steady
  system_names = u_sys
  l_tol = 1e-10
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
  execute_on = TIMESTEP_END
[]
