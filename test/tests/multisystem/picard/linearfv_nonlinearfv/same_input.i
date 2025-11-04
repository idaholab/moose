[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 6
  []
[]

[Problem]
  nl_sys_names = 'v_sys'
  linear_sys_names = 'u_sys'
[]

[Variables]
  [v]
    type = MooseVariableFVReal
    initial_condition = 2.0
    solver_sys = v_sys
  []
  [u]
    type = MooseLinearVariableFVReal
    solver_sys = 'u_sys'
    initial_condition = 1.0
  []
[]

[FVKernels]
  [diffusion]
    type = FVDiffusion
    variable = v
    coeff = u
  []
  [source]
    type = FVBodyForce
    variable = v
    function = 3
  []
[]

[LinearFVKernels]
  [diffusion]
    type = LinearFVDiffusion
    variable = u
    diffusion_coeff = v
  []
  [source]
    type = LinearFVSource
    variable = u
    source_density = 1
  []
[]

[FVBCs]
  [dir]
    type = FVFunctorDirichletBC
    variable = v
    boundary = "left right"
    functor = 2
  []
[]

[LinearFVBCs]
  [dir]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = "left right"
    functor = 1
  []
[]

[Convergence]
  [linear]
    type = IterationCountConvergence
    max_iterations = 6
    converge_at_max_iterations = true
  []
[]

[Executioner]
  type = Steady
  system_names = 'v_sys u_sys'
  l_abs_tol = 1e-12
  l_tol = 1e-10
  nl_abs_tol = 1e-10
  multi_system_fixed_point=true
  multi_system_fixed_point_convergence=linear
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
  execute_on = timestep_end
[]
