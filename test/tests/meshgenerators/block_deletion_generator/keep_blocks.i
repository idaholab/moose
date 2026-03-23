[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
  []

  [subdomains]
    type = SubdomainPerElementGenerator
    input = gmg
    # Note: this is a 1D vector per element ID
    subdomain_ids = '0 0 1 1
                     0 0 1 1
                     2 2 3 3
                     2 2 3 3'
  []

  [keep_blocks]
    type = BlockDeletionGenerator
    input = subdomains
    operation = keep
    block = '0 3'
  []

  # Subdomain assignment depend on element ids
  allow_renumbering = false
[]

[Outputs]
  exodus = true
[]
