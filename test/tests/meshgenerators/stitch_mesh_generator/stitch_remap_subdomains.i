[Mesh]
  [mesh1]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
    subdomain_ids = '0 0
                     1 1'
  []
  [name1]
    type = RenameBlockGenerator
    input = mesh1
    old_block = '0 1'
    new_block = 'fuel clad'
  []
  [mesh2]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
    xmin = 1
    xmax = 2
    subdomain_ids = '2 1
                     0 0'
  []
  # 'clad' has the same name and ID as in the first mesh: it should be kept as is
  # 'fuel' has ID 2 here and ID 0 in the first mesh: it should be merged with the first mesh 'fuel'
  # 'water' has ID 0 here, which is 'fuel' in the first mesh: it should get a new ID
  [name2]
    type = RenameBlockGenerator
    input = mesh2
    old_block = '2 1 0'
    new_block = 'fuel clad water'
  []
  [stitch]
    type = StitchMeshGenerator
    inputs = 'name1 name2'
    stitch_boundaries_pairs = 'right left'
    verbose_remapping = true
  []
[]
