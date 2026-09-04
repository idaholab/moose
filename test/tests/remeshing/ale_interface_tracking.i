[Mesh]
  [outer]
    type = PolyLineMeshGenerator
    points = '-1.0 -1.0 0.0
               1.0 -1.0 0.0
               1.0  1.0 0.0
              -1.0  1.0 0.0'
    loop = true
  []
  [circle]
    type = ParsedCurveGenerator
    x_formula = 'r*cos(t*2*pi)'
    y_formula = 'r*sin(t*2*pi)'
    section_bounding_t_values = '0 1'
    constant_names = 'pi r'
    constant_expressions = '${fparse pi} 0.3'
    nums_segments = '12'
    is_closed_loop = true
  []
  [domain]
    type = XYDelaunayGenerator
    boundary = 'outer'
    holes = 'circle'
    hole_boundaries = 'interface'
    refine_holes = 'false'
    refine_boundary = false
    add_nodes_per_boundary_segment = 3
    output_boundary = 'walls'
    desired_area = 0.1
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

[AuxVariables]
  [pos_x]
  []
  [pos_y]
  []
  [init_x]
    [InitialCondition]
      type = FunctionIC
      function = 'initial_x'
    []
  []
  [disp_x]
  []
[]

[AuxKernels]
  [sample_pos_x]
    type = ParsedAux
    variable = pos_x
    expression = 'x'
    use_xyzt = true
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [sample_pos_y]
    type = ParsedAux
    variable = pos_y
    expression = 'y'
    use_xyzt = true
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [sample_disp_x]
    type = ParsedAux
    variable = disp_x
    coupled_variables = 'init_x'
    expression = 'x - init_x'
    use_xyzt = true
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[BCs]
  [hot]
    type = DirichletBC
    variable = u
    boundary = 'interface'
    value = 1
  []
  [cold]
    type = DirichletBC
    variable = u
    boundary = 'walls'
    value = 0
  []
[]

[Functions]
  [initial_x]
    type = ParsedFunction
    expression = 'x'
  []
  [circle_vel_x]
    type = ConstantFunction
    value = 0.5
  []
  [circle_vel_y]
    type = ConstantFunction
    value = 0
  []
[]

[Remeshing]
  mesh_movement = true
  [Criteria]
    [quality]
      type = ElementQualityCriterion
      quality_metric = MIN_ANGLE
      threshold = 1e-3
    []
    [motion]
      type = MeshMotionCriterion
      threshold = 1e3
    []
  []
  [Remeshers]
    [patch]
      type = PatchDelaunayRemesher
    []
  []
  [Smoothers]
    [laplace]
      type = LaplaceSmoother
      moving_boundaries = 'interface'
      velocity_functions = 'circle_vel_x circle_vel_y'
    []
  []
[]

[Postprocessors]
  [interface_centroid_x]
    type = SideAverageValue
    variable = pos_x
    boundary = 'interface'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [interface_centroid_y]
    type = SideAverageValue
    variable = pos_y
    boundary = 'interface'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [interior_mean_disp_x]
    type = ElementAverageValue
    variable = disp_x
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [domain_centroid_y]
    type = ElementAverageValue
    variable = pos_y
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [wall_centroid_x]
    type = SideAverageValue
    variable = pos_x
    boundary = 'walls'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  num_steps = 3
  dt = 0.1
  solve_type = NEWTON
[]

[Outputs]
  csv = true
  [out]
    type = Exodus
    sequence = true
    execute_on = 'TIMESTEP_END'
  []
[]
