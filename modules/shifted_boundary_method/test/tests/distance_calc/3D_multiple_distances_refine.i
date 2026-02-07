nx = 10

[Problem]
  solve = false
[]

[Mesh]
  [boundary_mesh]
    type = FileMeshGenerator
    file = 'bunny.msh'
  []

  [shift_boundary_mesh]
    type = TransformGenerator
    transform = TRANSLATE
    vector_value = '2.0 2.0 1.5'
    input = boundary_mesh
    save_with_name = 'shift_boundary_mesh'
  []

  [gen]
    type = CartesianMeshGenerator
    dim = 3
    dx = '3'
    dy = '3'
    dz = '3'
    ix = '${nx}'
    iy = '${nx}'
    iz = '${nx}'
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
  []
  [distance_to_surface]
    type = ShortestDistanceToSurface
    surfaces = 'dist_bunny sphere1'
  []
[]

[Functions]
  [dist_bunny]
    type = UnsignedDistanceToSurfaceMesh
    builder = TreeBuilder
  []
  [sphere1]
    type = ParsedFunction
    expression = "sqrt((x-1.0)^2 + (y-1.0)^2 + (z-1.0)^2) - 0.5"
  []
[]

[Variables]
  [u]
    initial_condition = 1
    block = 1
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
    type = ElementCentroidToSurfaceDistanceAux
    distance_to_surface = distance_to_surface
    variable = distance
    execute_on = 'INITIAL timestep_begin'
  []
[]

[Adaptivity]
  marker = refine_based_on_aux
  max_h_level = 3
  steps = 2

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
