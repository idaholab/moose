[Mesh]
  [fine_mesh]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 12
    xmin = -12
    nx = 12
    ymax = 12
    ymin = -12
    ny = 12
    subdomain_ids = '
     0 0 0 0 1 1 1 1 2 2 2 2
     0 0 0 1 0 1 1 1 2 2 2 2
     0 0 0 0 1 1 1 1 2 2 2 2
     0 0 0 0 1 1 1 1 2 2 2 2
     3 3 3 3 4 4 4 4 5 5 5 5
     3 3 3 3 4 4 4 4 5 5 5 5
     3 3 3 4 3 4 4 4 5 5 5 5
     3 3 3 3 4 4 4 5 4 5 5 5
     6 6 6 6 7 7 7 7 8 8 10 10
     6 6 6 6 7 7 7 7 8 8 10 10
     6 6 6 6 7 7 7 7 8 8 8 8
     6 6 6 6 7 7 7 7 8 8 8 9
     '
  []
  [assign_elem_id]
    type = ParsedExtraElementIDGenerator
    input = fine_mesh
    extra_elem_integer_name = elem_id
    expression = 'if(sqrt(x*x+y*y)<=4, 4,
                  if(sqrt(x*x+y*y)<=6, 6,
                  if(sqrt(x*x+y*y)<=8, 8,
                  if(sqrt(x*x+y*y)<=10, 10, 12))))'
  []
  [assign_elem_id2]
    type = ParsedExtraElementIDGenerator
    input = assign_elem_id
    restricted_subdomains = '4 5 6 7'
    extra_elem_integer_name = elem_id2
    extra_element_id_names = elem_id
    expression = 'elem_id+1'
  []
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  [out]
    type = Exodus
    output_extra_element_ids = true
  []
[]
