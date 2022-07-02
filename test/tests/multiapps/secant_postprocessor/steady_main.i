[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
  parallel_type = replicated
  uniform_refine = 1
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [source]
    type = BodyForce
    variable = u
    value = 1
  []
[]

[BCs]
  [left]
    type = PostprocessorDirichletBC
    variable = u
    boundary = left
    postprocessor = 'from_sub'
  []
[]

[Postprocessors]
  [from_sub]
    type = Receiver
    default = 0
  []
  [to_sub]
    type = SideAverageValue
    variable = u
    boundary = right
  []
  [average]
    type = ElementAverageValue
    variable = u
  []
[]

[Executioner]
  type = Steady

  # Solve parameters
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  nl_abs_tol = 1e-14

  # App coupling parameters
  fixed_point_algorithm = 'secant'
  fixed_point_max_its = 100
  transformed_postprocessors = 'from_sub'
[]

[Outputs]
  csv = true
  exodus = false
[]

[MultiApps]
  [sub]
    type = FullSolveMultiApp
    app_type = MooseTestApp
    positions = '0 0 0'
    input_files = steady_sub.i
    clone_parent_mesh = true
    execute_on = 'timestep_begin'
    # we keep the full postprocessor output history of the subapp
    keep_full_output_history = true

    transformed_postprocessors = 'from_main'
  []
[]

[Transfers]
  [left_from_sub]
    type = MultiAppPostprocessorTransfer
    from_multi_app = sub
    from_postprocessor = 'to_main'
    to_postprocessor = 'from_sub'
    reduction_type = 'average'
  []
  [right_to_sub]
    type = MultiAppPostprocessorTransfer
    to_multi_app = sub
    from_postprocessor = 'to_sub'
    to_postprocessor = 'from_main'
  []
[]
