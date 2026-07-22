[Problem]
  solve = false
[]

[Mesh]
  [boundary_mesh]
    type = FileMeshGenerator
    file = '../distance_calc/square_boundary.msh'
  []
  [shift_boundary_mesh]
    type = TransformGenerator
    transform = TRANSLATE
    vector_value = '1.5 1.5 0'
    input = boundary_mesh
    save_with_name = 'shift_boundary_mesh'
  []
  [background_mesh]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 4
    ymin = 0
    ymax = 4
    nx = 4
    ny = 4
    subdomain_ids = 7
  []
  final_generator = background_mesh
  add_subdomain_ids = 0
[]

[UserObjects]
  [surface_builder]
    type = SurfaceMeshBySubdomainBuilder
    surface_mesh = shift_boundary_mesh
  []
  [subdomain_tester]
    type = PointInSubdomainCheckUO
    brute_force = true
    builder = surface_builder
  []
[]

[MeshModifiers]
  [assign_subdomains]
    type = SubdomainElementModifier
    subdomain_id_tester = subdomain_tester
    lambda = 0.5
    execute_on = INITIAL
  []
[]

[Reporters]
  [mesh_info]
    type = MeshInfo
    items = subdomain_elems
    outputs = json
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  [json]
    type = JSON
  []
[]
