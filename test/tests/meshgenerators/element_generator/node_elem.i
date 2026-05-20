[Mesh]
  [mgA]
    type = ElementGenerator
    elem_type = NODEELEM
    nodal_positions = '1 2 3'
    element_connectivity = '0'
    subdomain_id = 0
    subdomain_name = blockA
  []
  [mgB]
    type = ElementGenerator
    elem_type = NODEELEM
    nodal_positions = '4 5 6'
    element_connectivity = '0'
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
