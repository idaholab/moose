[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 6
  []
[]

[Variables]
  [v]
    type = MooseVariableFVReal
    initial_condition = 2.0
  []
[]

[AuxVariables]
  [diff_var]
    type = MooseVariableFVReal
    initial_condition = 1.0
  []
[]

[FVKernels]
  [diffusion]
    type = FVDiffusion
    variable = v
    coeff = diff_var
  []
  [source]
    type = FVBodyForce
    variable = v
    function = 3
  []
[]

[MultiApps]
  inactive = 'linear'
  [linear]
    type = FullSolveMultiApp
    input_files = linearfv.i
    execute_on = timestep_begin
    no_restore = true
  []
[]

[Transfers]
  inactive = 'from_linear to_linear'
  [from_linear]
    type = MultiAppCopyTransfer
    from_multi_app = linear
    source_variable = 'u'
    variable = 'diff_var'
    execute_on = timestep_begin
  []
  [to_linear]
    type = MultiAppCopyTransfer
    to_multi_app = linear
    source_variable = 'v'
    variable = 'diff_var'
    execute_on = timestep_begin
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

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  nl_abs_tol = 1e-12
  fixed_point_rel_tol = 1e-10
[]

[Outputs]
  exodus = true
  execute_on = timestep_end
[]
