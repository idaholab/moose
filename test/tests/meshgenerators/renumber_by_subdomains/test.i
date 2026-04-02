[Mesh]
  # CartesianMG numbers in XYZ
  # But we'll place subdomains alternating and in a different order
  # so the renumbering modifies it
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '2 2'
    dy = '3 2'
    ix = '3 2'
    iy = '2 4'
    subdomain_id = '1 0
                    3 2'
  []
  [rename]
    type = RenameBlockGenerator
    input = 'cmg'
    old_block = '0 1 2 3'
    new_block = 'zero one two three'
  []
  [renumber]
    type = RenumberBySubdomainGenerator
    input = 'rename'
    blocks_to_renumber = 'one two zero three'
  []
[]
