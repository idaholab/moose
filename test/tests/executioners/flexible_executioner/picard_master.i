[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
  []
  parallel_type = replicated
  uniform_refine = 1
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [v]
  []
[]

[Kernels]
  [diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
  []
  [force_u]
    type = CoupledForce
    variable = u
    v = v
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Postprocessors]
  [picard_its]
    type = NumFixedPointIterations
    execute_on = 'initial timestep_end'
  []
[]

[Executioner]
  type = FlexibleExecutioner
  solve_object_ordering = 'picard solve'
  [solve]
    type = FEProblemSolve
    solve_type = PJFNK
    petsc_options_iname = '-pc_type -pc_hypre_type'
    petsc_options_value = 'hypre boomeramg'
    nl_abs_tol = 1e-14
  []
  [picard]
    type = PicardSolve
    fixed_point_max_its = 100
    fixed_point_abs_tol = 1e-8
  []
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [sub]
    type = FullSolveMultiApp
    input_files = picard_sub.i
    clone_master_mesh = true
  []
[]

[Transfers]
  [v_from_sub]
    type = MultiAppNearestNodeTransfer
    direction = from_multiapp
    multi_app = sub
    source_variable = v
    variable = v
  []
  [u_to_sub]
    type = MultiAppNearestNodeTransfer
    direction = to_multiapp
    multi_app = sub
    source_variable = u
    variable = u
  []
[]
