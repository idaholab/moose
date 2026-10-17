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
      moving_boundaries = 'interface'
      velocity_functions = 'circle_vel_x circle_vel_y'
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
