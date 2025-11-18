nx = 64
[Problem]
  solve = false
[]

[Mesh]
  [boundary_mesh]
    type = FileMeshGenerator
    file = 'star.msh'
  []

  [shift_boundary_mesh]
    type = TransformGenerator
    transform = TRANSLATE
    vector_value = '2.0 2.0 0.0' # translation in x, y, z directions
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

  add_subdomain_ids = 2 # outside block
  add_sideset_names = 'SBMinterface'
  final_generator = 'gen'
[]

[UserObjects]
  [TreeBuilder]
    type = SBMSurfaceMeshBuilder
    surface_mesh = shift_boundary_mesh
  []
  [distance_to_surface]
    type = ShortestDistanceToSurface
    surfaces = 'dist_star circle1 ellipse_rot'
  []
[]

[Variables]
  [u]
    initial_condition = 1
    block = 1
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
  [ellipse_rot]
    type = ParsedFunction
    symbol_names = 'a b theta cx cy'
    symbol_values = '0.8 0.4 0.785398 3.0 1.0'
    expression = "
      (
        (
          ((x-cx)*cos(theta) + (y-cy)*sin(theta)) / a
        )^2
        +
        (
          (-(x-cx)*sin(theta) + (y-cy)*cos(theta)) / b
        )^2
      )^(0.5) - 1
    "
  []
[]

[AuxVariables]
  [distance]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [dist]
    type = DistanceAux
    distance_to_surface = distance_to_surface
    variable = distance
    execute_on = 'INITIAL timestep_begin'
  []
[]

[Adaptivity]
  marker = refine_based_on_aux
  max_h_level = 5
  steps = 3
  [Markers]
    [refine_based_on_aux]
      type = ValueThresholdMarker
      variable = distance
      refine = 0.05
      coarsen = 0.5
      invert = true
    []
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
