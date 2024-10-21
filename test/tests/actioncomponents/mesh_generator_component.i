[Mesh]
  [cylinder_1]
    type = CartesianMeshGenerator
    dim = 2
    ix = 1
    iy = 1
    dx = 2
    dy = 10
  []
  [move_down]
    type = TransformGenerator
    input = 'cylinder_1'
    transform = 'TRANSLATE'
    vector_value = '0 -11 0'
  []
  [cmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 1
    xmax = 1
    ymax = 4
    subdomain_name = cyl_2
    save_with_name = 'saved_mesh_0'

    # We can't have names overlapping (likely by accident) between meshes from [Mesh]
    # and from the [Components] block
    boundary_name_prefix = 'cyl'
  []
  final_generator = 'move_down'
[]

[ActionComponents]
  # Note that we choose the parameters of the mesh generator to match the
  # 'two_same_components.i' output
  [cylinder_2]
    type = MeshGeneratorComponent
    mesh_generator = 'cmg'
    mesh_generator_type = 'saved_mesh'
    saved_mesh_name = 'saved_mesh_0'
  []
[]

[Problem]
  skip_nl_system_check = true
[]

[Executioner]
  type = Steady
[]

[AuxVariables]
  [dummy]
  []
[]

[Outputs]
  exodus = true
[]
