[Mesh]
  [two_domains]
    type = CartesianMeshGenerator
    dim = 2
    dx = '5 5'
    dy = '1 1'
    ix = '1 1'
    iy = '1 2'
    subdomain_id = '0 1
                    1 1'
  []
  [repartition_one_domain_into_two_more]
    type = SubdomainsFromPartitionerGenerator
    input = two_domains
    included_subdomains = '1'
    offset = 3
    # Note: most partitioners will require the number of ranks to match this
    num_partitions = 2
  []

  [Partitioner]
    type = LibmeshPartitioner
    partitioner = 'centroid'
    centroid_partitioner_direction = 'x'
  []
[]
