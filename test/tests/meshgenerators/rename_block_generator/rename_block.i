[Mesh]
  [gmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1'
    dy = '1 1'
    ix = '2 2'
    iy = '2 2'
    subdomain_id = '0 1 2 3'
  []

  [set_names]
    type = RenameBlockGenerator
    old_block = '0 1 2 3'
    new_block = 'block0 block1 block2 block3'
    input = gmg
  []

  # Rename parameters supplied through the "tests" specifications
  [rename]
    type = RenameBlockGenerator
    input = set_names
  []

  # We compare by element numbers, which are not consistent in parallel
  # if this is true
  allow_renumbering = false
[]

[Reporters/mesh_info]
  type = MeshInfo
  items = subdomain_elems
[]

[Outputs/out]
  type = JSON
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
