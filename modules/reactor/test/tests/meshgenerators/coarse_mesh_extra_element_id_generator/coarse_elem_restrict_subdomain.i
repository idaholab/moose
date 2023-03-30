[Mesh]
  [fine_mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '0.5 0.5'
    dy = '0.5 0.5'
    ix = '5 5'
    iy = '5 5'
    subdomain_id = '0 1
                    0 1'
  []
  [coarse_mesh_x]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 10
  []
  [coarse_mesh_y]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 1
  []
  [assign1]
    type = CoarseMeshExtraElementIDGenerator
    input = fine_mesh
    coarse_mesh = coarse_mesh_x
    extra_element_id_name = 'coarse_elem_id'
    subdomains = 0
  []
  [assign2]
    type = CoarseMeshExtraElementIDGenerator
    input = assign1
    coarse_mesh = coarse_mesh_y
    extra_element_id_name = 'coarse_elem_id'
    subdomains = 1
  []
  allow_renumbering = false
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[AuxVariables]
  [coarse_elem_id]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [coarse_elem_id]
    type = ExtraElementIDAux
    variable = coarse_elem_id
    extra_id_name = coarse_elem_id
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
