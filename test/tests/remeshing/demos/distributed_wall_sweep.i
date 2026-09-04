[Mesh]
  parallel_type = DISTRIBUTED
  displacements = 'disp_x disp_y'
  [square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 24
    ny = 24
    elem_type = TRI3
  []
  [smooth]
    type = SmoothMeshGenerator
    input = square
    algorithm = variational
    verbosity = 0
  []
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
  [shape]
    family = MONOMIAL
    order = CONSTANT
  []
  [target_h]
    family = MONOMIAL
    order = CONSTANT
    # Cap on the longest element edge, roughly twice the initial mesh size, so the splitter only
    # fires where the boundary sliding has stretched an edge well past the target spacing
    initial_condition = 0.09
  []
[]

[AuxKernels]
  [shape_aux]
    type = ElementQualityAux
    variable = shape
    metric = SHAPE
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [time]
    type = TimeDerivative
    variable = u
  []
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
    [quality]
      type = ElementQualityCriterion
      quality_metric = MIN_ANGLE
      threshold = 25
    []
  []
  [Remeshers]
    # The splitter runs first: it may insert nodes on over-long external boundary edges, which the
    # patch remesher may not touch, and the patch pass then cleans up the shapes
    [split]
      type = TriSplitRemesher
      sizing_variable = target_h
    []
    [patch]
      type = PatchDelaunayRemesher
      quality_lower_bound = 0.5
      # Uniform target area (the initial 24x24 right-triangle size) so the patches are not sized
      # off the crowded boundary spacing near the travelling corner, which cascades into runaway
      # refinement and boundary slivers
      desired_area = 8.7e-4
      # Drop crowded collinear boundary nodes, which is what keeps the travelling corner from
      # piling boundary spacing up against the pinned corners and forcing slivers
      coarsen_boundary_fraction = 0.6
    []
  []
  [Smoothers]
    [variational]
      type = VariationalSmoother
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
  num_steps = 30
  dt = 0.1
  solve_type = NEWTON
  nl_abs_tol = 1e-10
[]

[Outputs]
  csv = true
  [exo]
    type = Nemesis
  []
[]
