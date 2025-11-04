[Mesh]
  [bulk]
    type = CartesianMeshGenerator
    dim = 3
    dx = '0.5 0.75 0.025 4.0 0.025 0.75 0.5'
    dy = '0.5 0.3 0.025 7.6 0.025 0.75 0.5'
    dz = '0.5 0.3 0.025 3.6 0.025 0.3 0.5'
    ix = '2 3 1 16 1 3 2'
    iy = '2 1 1 30 1 3 2'
    iz = '2 1 1 14 1 1 2'
    subdomain_id = '
      0 0 0 0 0 0 0
      0 0 0 0 0 0 0
      0 0 0 0 0 0 0
      0 0 0 0 0 0 0
      0 0 0 0 0 0 0
      0 0 0 0 0 0 0
      0 0 0 0 0 0 0

      0 0 0 0 0 0 0
      0 1 1 1 1 1 0
      0 2 2 1 2 2 0
      0 2 2 1 2 2 0
      0 2 2 2 2 2 0
      0 2 2 2 2 2 0
      0 0 0 0 0 0 0

      0 0 0 0 0 0 0
      0 1 1 1 1 1 0
      0 2 2 3 2 2 0
      0 2 2 3 2 2 0
      0 2 2 2 2 2 0
      0 2 2 2 2 2 0
      0 0 0 0 0 0 0

      0 0 0 0 0 0 0
      0 1 1 1 1 1 0
      0 2 2 3 2 2 0
      0 2 2 4 2 2 0
      0 2 2 2 2 2 0
      0 2 2 2 2 2 0
      0 0 0 0 0 0 0

      0 0 0 0 0 0 0
      0 1 1 1 1 1 0
      0 2 2 3 2 2 0
      0 2 2 3 2 2 0
      0 2 2 2 2 2 0
      0 2 2 2 2 2 0
      0 0 0 0 0 0 0

      0 0 0 0 0 0 0
      0 1 1 1 1 1 0
      0 1 1 1 1 1 0
      0 1 1 1 1 1 0
      0 1 1 1 1 1 0
      0 1 1 1 1 1 0
      0 0 0 0 0 0 0

      0 0 0 0 0 0 0
      0 0 0 0 0 0 0
      0 0 0 0 0 0 0
      0 0 0 0 0 0 0
      0 0 0 0 0 0 0
      0 0 0 0 0 0 0
      0 0 0 0 0 0 0
      '
  []
  [hollow_concrete]
    type = BlockDeletionGenerator
    input = bulk
    block = 4
  []
  [rename_blocks]
    type = RenameBlockGenerator
    input = hollow_concrete
    old_block = '0 1 2 3'
    new_block = 'concrete_hd concrete water Al'
    show_info = true
  []
  [rename_boundaries_step1]
    type = RenameBoundaryGenerator
    input = 'rename_blocks'
    old_boundary = 'back  front'
    new_boundary = 'temp1 temp2'
    show_info = true
  []
  [rename_boundaries_step2]
    type = RenameBoundaryGenerator
    input = 'rename_boundaries_step1'
    old_boundary = 'bottom top'
    new_boundary = 'back   front'
    show_info = true
  []
  [rename_boundaries_step3]
    type = RenameBoundaryGenerator
    input = 'rename_boundaries_step2'
    old_boundary = 'temp1 temp2'
    new_boundary = 'bottom top'
  []
[]
