nx = 16

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
    vector_value = '2.0 2.0 2.0'
    input = boundary_mesh
    save_with_name = 'shift_boundary_mesh'
  []

  [gen]
    type = CartesianMeshGenerator
    dim = 3
    dx = '4'
    dy = '4'
    dz = '4'
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
    surfaces = 'dist_bunny'
  []
[]

[Functions]
  [dist_bunny]
    type = UnsignedDistanceToSurfaceMesh
    builder = TreeBuilder
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
    type = DistanceAux
    distance_to_surface = distance_to_surface
    variable = distance
    execute_on = 'INITIAL timestep_begin'
  []
[]

[Adaptivity]
  marker = refine_based_on_aux
  max_h_level = 8
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
