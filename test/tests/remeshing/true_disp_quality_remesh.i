[Mesh]
  displacements = 'ux uy'
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
  [ux]
  []
  [uy]
  []
[]

[Kernels]
  [ux_time]
    type = TimeDerivative
    variable = ux
  []
  [ux_diff]
    type = Diffusion
    variable = ux
  []
  [uy_time]
    type = TimeDerivative
    variable = uy
  []
  [uy_diff]
    type = Diffusion
    variable = uy
  []
[]

[Functions]
  [push_x]
    type = ParsedFunction
    expression = '0.7*t'
  []
  [push_y]
    type = ParsedFunction
    expression = '0.2*t'
  []
[]

[BCs]
  [ux_interface]
    type = FunctionDirichletBC
    variable = ux
    boundary = 'interface'
    function = push_x
  []
  [uy_interface]
    type = FunctionDirichletBC
    variable = uy
    boundary = 'interface'
    function = push_y
  []
  [ux_walls]
    type = DirichletBC
    variable = ux
    boundary = 'walls'
    value = 0
  []
  [uy_walls]
    type = DirichletBC
    variable = uy
    boundary = 'walls'
    value = 0
  []
[]

[Remeshing]
  displacements = 'ux uy'
  [Criteria]
    [quality]
      type = ElementQualityCriterion
      quality_metric = MIN_ANGLE
      threshold = 15
    []
  []
  [Remeshers]
    [patch]
      type = PatchDelaunayRemesher
      quality_lower_bound = 0.9
    []
  []
[]

[Postprocessors]
  [remesh_count]
    type = RemeshCount
    execute_on = 'INITIAL TIMESTEP_END FINAL'
  []
  [n_elements]
    type = NumElements
    execute_on = 'INITIAL TIMESTEP_END FINAL'
  []
[]

[Executioner]
  type = Transient
  num_steps = 8
  dt = 0.1
  solve_type = NEWTON
[]

[Outputs]
  [out]
    type = Exodus
    execute_on = 'FINAL'
  []
[]
