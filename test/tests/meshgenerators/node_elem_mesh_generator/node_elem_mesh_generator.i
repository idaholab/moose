[Mesh]
  [mgA]
    type = NodeElemMeshGenerator
    point = '1 2 3'
    subdomain_id = 0
    subdomain_name = blockA
  []
  [mgB]
    type = NodeElemMeshGenerator
    point = '4 5 6'
    subdomain_id = 1
    subdomain_name = blockB
  []
  [combiner]
    type = CombinerGenerator
    inputs = 'mgA mgB'
  []
[]

[Outputs]
  exodus = true
[]
