[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 6
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
  [diff_var]
    type = MooseVariableFVReal
    initial_condition = 2.0
  []
[]

[LinearFVKernels]
  [diffusion]
    type = LinearFVDiffusion
    variable = u
    diffusion_coeff = diff_var
  []
  [source]
    type = LinearFVSource
    variable = u
    source_density = 1
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

[MultiApps]
  inactive = 'nonlinear'
  [nonlinear]
    type = FullSolveMultiApp
    input_files = nonlinearfv.i
    execute_on = timestep_begin
    no_restore = true
  []
[]

[Transfers]
  inactive = 'from_nonlinear to_nonlinear'
  [from_nonlinear]
    type = MultiAppCopyTransfer
    from_multi_app = nonlinear
    source_variable = 'v'
    variable = 'diff_var'
    execute_on = timestep_begin
  []
  [to_nonlinear]
    type = MultiAppCopyTransfer
    to_multi_app = nonlinear
    source_variable = 'u'
    variable = 'diff_var'
    execute_on = timestep_begin
  []
[]

[Executioner]
  type = Steady
  system_names = u_sys
  l_abs_tol = 1e-12
  l_tol = 1e-10
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  fixed_point_rel_tol = 1e-10
[]

[Outputs]
  exodus = true
  execute_on = timestep_end
[]
