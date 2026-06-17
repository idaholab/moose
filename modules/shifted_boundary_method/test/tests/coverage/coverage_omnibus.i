# Coverage driver for ShortestDistanceToSurface's trueNormal / *ByIndex / *ByFunc
# accessors (via the test-only ShortestDistanceToSurfaceTestAux), the
# zero-gradient short-circuits in SBMUtils (via a constant ParsedFunction), and
# the watertight-mesh log branch in SBMSurfaceMeshBuilder.

nx = 16

[Problem]
  solve = false
[]

[Mesh]
  [boundary_mesh]
    type = FileMeshGenerator
    file = '../distance_calc/star.msh'
  []
  [shift_boundary_mesh]
    type = TransformGenerator
    transform = TRANSLATE
    vector_value = '2.0 2.0 0.0'
    input = boundary_mesh
    save_with_name = 'shift_boundary_mesh'
  []
  [gen]
    type = CartesianMeshGenerator
    dim = 2
    dx = '4'
    dy = '4'
    ix = '${nx}'
    iy = '${nx}'
    subdomain_id = '1'
  []
  add_subdomain_ids = 2
  add_sideset_names = 'SBMinterface'
  final_generator = 'gen'
[]

[UserObjects]
  [TreeBuilder]
    type = SBMSurfaceMeshBuilder
    surface_mesh = shift_boundary_mesh
    check_watertightness = true # exercises watertight=true log branch
  []
  [distance_to_surface]
    type = ShortestDistanceToSurface
    surfaces = 'dist_star circle1 const_one'
  []
[]

[Functions]
  [dist_star]
    type = UnsignedDistanceToSurfaceMesh
    builder = TreeBuilder
  []
  [circle1]
    type = ParsedFunction
    expression = "sqrt((x-1.0)^2 + (y-1.0)^2) - 0.5"
  []
  # Constant function: value()=1, gradient()=0. Triggers the
  # zero-gradient short-circuits in SBMUtils.
  [const_one]
    type = ParsedFunction
    expression = "1.0"
  []
[]

[AuxVariables]
  [dist_centroid]
    order = CONSTANT
    family = MONOMIAL
  []
  [true_normal_x]
    order = CONSTANT
    family = MONOMIAL
  []
  [dist_by_index]
    order = CONSTANT
    family = MONOMIAL
  []
  [true_normal_by_index]
    order = CONSTANT
    family = MONOMIAL
  []
  # Distance/normal via *ByFunc using the mesh-based UnsignedDistance function.
  [dist_by_func_mesh]
    order = CONSTANT
    family = MONOMIAL
  []
  [normal_by_func_mesh]
    order = CONSTANT
    family = MONOMIAL
  []
  # Distance/normal via *ByFunc using the zero-gradient constant function.
  # Forces the zero-gradient branches in SBMUtils to fire.
  [dist_by_func_const]
    order = CONSTANT
    family = MONOMIAL
  []
  [normal_by_func_const]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [dist_centroid]
    type = ElementCentroidToSurfaceDistanceAux
    distance_to_surface = distance_to_surface
    variable = dist_centroid
    execute_on = 'INITIAL'
  []
  [true_normal_x]
    type = ShortestDistanceToSurfaceTestAux
    distance_to_surface = distance_to_surface
    method = true_normal
    component = x
    variable = true_normal_x
    execute_on = 'INITIAL'
  []
  [dist_by_index]
    type = ShortestDistanceToSurfaceTestAux
    distance_to_surface = distance_to_surface
    method = distance_by_index
    index = 1 # circle1
    variable = dist_by_index
    execute_on = 'INITIAL'
  []
  [true_normal_by_index]
    type = ShortestDistanceToSurfaceTestAux
    distance_to_surface = distance_to_surface
    method = true_normal_by_index
    index = 1 # circle1 (ParsedFunction branch of trueNormalFromFunction)
    component = x
    variable = true_normal_by_index
    execute_on = 'INITIAL'
  []
  [dist_by_func_mesh]
    type = ShortestDistanceToSurfaceTestAux
    distance_to_surface = distance_to_surface
    method = distance_by_func
    function = dist_star # exercises UnsignedDistanceToSurfaceMesh branch
    variable = dist_by_func_mesh
    execute_on = 'INITIAL'
  []
  [normal_by_func_mesh]
    type = ShortestDistanceToSurfaceTestAux
    distance_to_surface = distance_to_surface
    method = true_normal_by_func
    function = dist_star # mesh-function branch of trueNormalFromFunction
    component = x
    variable = normal_by_func_mesh
    execute_on = 'INITIAL'
  []
  [dist_by_func_const]
    type = ShortestDistanceToSurfaceTestAux
    distance_to_surface = distance_to_surface
    method = distance_by_func
    function = const_one # zero-gradient ParsedFunction branch
    variable = dist_by_func_const
    execute_on = 'INITIAL'
  []
  [normal_by_func_const]
    type = ShortestDistanceToSurfaceTestAux
    distance_to_surface = distance_to_surface
    method = true_normal_by_func
    function = const_one # zero-gradient parsed branch in trueNormalFromFunction
    component = x
    variable = normal_by_func_const
    execute_on = 'INITIAL'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
