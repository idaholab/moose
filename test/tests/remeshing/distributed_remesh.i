[Mesh]
  parallel_type = DISTRIBUTED
  displacements = 'disp_x disp_y'
  [square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 12
    ny = 12
    elem_type = TRI3
  []
  # Pinned on purpose, see the 'distributed_remesh' block of the tests spec
  [Partitioner]
    type = GridPartitioner
    nx = 1
    ny = 3
    nz = 1
  []
[]

[AuxVariables]
  [disp_x]
  []
  [disp_y]
  []
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
[]

[BCs]
  [hot]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 1
  []
  [cold]
    type = DirichletBC
    variable = u
    boundary = 'right'
    value = 0
  []
[]

[Functions]
  [wall_vel_x]
    type = ParsedFunction
    expression = '0.3*y'
  []
  [wall_vel_y]
    type = ConstantFunction
    value = 0
  []
[]

[Remeshing]
  mesh_movement = true
  [Criteria]
    [motion]
      type = MeshMotionCriterion
      threshold = 0.35
    []
  []
  [Remeshers]
    [patch]
      type = PatchDelaunayRemesher
      quality_lower_bound = 0.9
    []
  []
  [Smoothers]
    [laplace]
      type = LaplaceSmoother
      moving_boundaries = 'left'
      velocity_functions = 'wall_vel_x wall_vel_y'
    []
  []
[]

[Postprocessors]
  [remesh_count]
    type = RemeshCount
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [max_pseudo_displacement]
    type = MaxPseudoDisplacement
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [n_elements]
    type = NumElements
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [u_l2]
    type = ElementL2Norm
    variable = u
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [displaced_volume]
    type = VolumePostprocessor
    use_displaced_mesh = true
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  num_steps = 8
  dt = 0.1
  solve_type = NEWTON
[]

[Outputs]
  csv = true
[]
