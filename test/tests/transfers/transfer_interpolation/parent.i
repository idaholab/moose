[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  # This test currently diffs when run in parallel with DistributedMesh enabled,
  # most likely due to the fact that it uses some geometric search stuff.
  # For more information, see #2121.
  parallel_type = replicated
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[UserObjects]
  [./layered_average]
    type = LayeredAverage
    variable = u
    direction = x
    num_layers = 3
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 1
  nl_rel_tol = 1e-10

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
[MultiApps]
  [./sub]
    type = TransientMultiApp
    app_type = MooseTestApp
    execute_on = timestep_end
    positions = '0 0 0'
    input_files = sub.i
    sub_cycling = true
    interpolate_transfers = true
    output_sub_cycles = true
  [../]
[]

[Transfers]
  [./nearest_node]
    type = MultiAppNearestNodeTransfer
    to_multi_app = sub
    source_variable = u
    variable = nearest_node
  [../]
  [./mesh_function]
    type = MultiAppShapeEvaluationTransfer
    to_multi_app = sub
    source_variable = u
    variable = mesh_function
  [../]
  [./user_object]
    type = MultiAppUserObjectTransfer
    to_multi_app = sub
    variable = user_object
    user_object = layered_average
  [../]
  [./interpolation]
    type = MultiAppGeometricInterpolationTransfer
    to_multi_app = sub
    source_variable = u
    variable = interpolation
  [../]
[]
