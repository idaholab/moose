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
  [time]
    type = TimeDerivative
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
  [coupling_its]
    type = NumFixedPointIterations
    execute_on = 'initial timestep_end'
  []
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
  type = Transient
  num_steps = 10
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  nl_abs_tol = 1e-14

  # App coupling parameters
  fixed_point_max_its = 30
  relaxation_factor = 0.8
  transformed_postprocessors = 'from_sub'
[]

[Outputs]
  csv = true
  exodus = false
[]

[MultiApps]
  [sub]
    type = TransientMultiApp
    app_type = MooseTestApp
    positions = '0 0 0'
    input_files = 'transient_sub.i'
    clone_parent_mesh = true
    execute_on = 'timestep_begin'
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
