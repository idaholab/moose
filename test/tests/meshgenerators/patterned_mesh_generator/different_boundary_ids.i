[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
    subdomain_ids = '1 1 1 1 1
                     1 1 1 1 1
                     1 1 1 1 1
                     1 1 1 1 1
                     1 1 1 1 1
                     '
  []

  [gmg2]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
    subdomain_ids = '2 2 2 2 2
                     2 2 2 2 2
                     2 2 2 2 2
                     2 2 2 2 2
                     2 2 2 2 2
                     '
    # The following triggers generation of new common boundary ids in
    # PatternedMeshGenerator
    boundary_id_offset = 1
  []

  [pmg]
    type = PatternedMeshGenerator
    inputs = 'gmg gmg2'
    pattern = '1 0 ;
               0 1'
  []
[]

[Outputs]
  exodus = true
[]
