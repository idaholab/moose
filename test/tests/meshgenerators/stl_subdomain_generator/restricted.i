[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 4
    ny = 4
    nz = 4
  []

  [seed]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    bottom_left = '0 0 0'
    top_right = '0.5 1 1'
    block_id = 2
  []

  [stl]
    type = STLSubdomainGenerator
    input = seed
    stl_file = cube_ascii.stl
    translation = '0.5 0.5 0.5'
    restricted_subdomains = '2'
    block_id = 1
  []
[]

[VectorPostprocessors]
  [elem_counter]
    type = ElementCounterWithID
    id_name = subdomain_id
  []
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
  execute_on = timestep_end
[]
