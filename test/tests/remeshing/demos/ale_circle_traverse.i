[Mesh]
  [outer]
    type = PolyLineMeshGenerator
    points = '-2.0 -0.6 0.0
               2.0 -0.6 0.0
               2.0  0.6 0.0
              -2.0  0.6 0.0'
    loop = true
  []
  [circle]
    type = ParsedCurveGenerator
    x_formula = 'x0 + r*cos(t*2*pi)'
    y_formula = 'r*sin(t*2*pi)'
    section_bounding_t_values = '0 1'
    constant_names = 'pi r x0'
    constant_expressions = '${fparse pi} 0.2 -1.4'
    nums_segments = '16'
    is_closed_loop = true
  []
  [domain]
    type = XYDelaunayGenerator
    boundary = 'outer'
    holes = 'circle'
    hole_boundaries = 'interface'
    refine_holes = 'false'
    refine_boundary = true
    add_nodes_per_boundary_segment = 3
    output_boundary = 'walls'
    desired_area = 0.01
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
    outputs = exo
    output_properties = 'thermal_conductivity'
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
    type = ParsedFunction
    expression = 'if(t < 5.5, 0.5, -0.5)'
  []
  [circle_vel_y]
    type = ConstantFunction
    value = 0
  []
[]

[Remeshing]
  mesh_movement = true
  check_interval = 1
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
    [patch]
      type = PatchDelaunayRemesher
      quality_metric = SHAPE
      quality_lower_bound = 0.5
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

[AuxVariables]
  [shape]
    family = MONOMIAL
    order = CONSTANT
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

[Postprocessors]
  [min_shape]
    type = ElementExtremeValue
    variable = shape
    value_type = min
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [state_integral]
    type = ElementIntegralMaterialProperty
    mat_prop = thermal_conductivity
    execute_on = 'TIMESTEP_END'
  []
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
  num_steps = 110
  dt = 0.1
  solve_type = NEWTON
  nl_abs_tol = 1e-10
[]

[Outputs]
  csv = true
  [exo]
    type = Exodus
    sequence = true
  []
[]
