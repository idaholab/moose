[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 4
    ny = 4
    nz = 4
  []

  [stl]
    type = STLSubdomainGenerator
    input = gmg
    stl_file = cube_ascii.stl
    translation = '0.5 0.5 0.5'
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
