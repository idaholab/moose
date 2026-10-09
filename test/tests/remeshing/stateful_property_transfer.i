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

[ProjectedStatefulMaterialStorage]
  [state]
    projected_props = 'thermal_conductivity'
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Materials]
  [stateful]
    type = StatefulSpatialTest
    use_interpolated_state = true
  []
[]

[Remeshing]
  check_interval = 4
  [Criteria]
    [quality]
      type = ElementQualityCriterion
      quality_metric = MIN_ANGLE
      threshold = 60
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
  [state_integral]
    type = ElementIntegralMaterialProperty
    mat_prop = thermal_conductivity
    execute_on = 'TIMESTEP_END'
  []
  [remesh_count]
    type = RemeshCount
    execute_on = 'TIMESTEP_END'
  []
  [n_elements]
    type = NumElements
    execute_on = 'TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  num_steps = 8
  dt = 0.1
  solve_type = NEWTON
  nl_abs_tol = 1e-10
[]

[Outputs]
  csv = true
  execute_on = 'TIMESTEP_END'
[]
