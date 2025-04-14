[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 1
    xmin = -1
    ymax = 1
    ymin = -1
    nx = 2
    ny = 2
  []
  [add_eeid]
    type = ParsedExtraElementIDGenerator
    input = gmg
    expression = 'if(x>0,if(y>0,1,4),if(y>0,2,3))'
    extra_elem_integer_name = 'eeid'
  []
  [psidg]
    type = ParsedSubdomainIDsGenerator
    input = add_eeid
    expression = 'if(x>0,if(y>0,1.2,3.8),if(y>0,2.3,2.9))'
  []
[]
