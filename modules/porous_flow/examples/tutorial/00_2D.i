# Creates the mesh for the remainder of the tutorial
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    xmin = 1.0
    xmax = 10
    bias_x = 1.4
    ny = 3
    ymin = -6
    ymax = 6
  []
  [aquifer]
    type = SubdomainBoundingBoxGenerator
    block_id = 1
    bottom_left = '0 -2 0'
    top_right = '10 2 0'
    input = gen
  []
  [injection_area]
    type = ParsedGenerateSideset
    combinatorial_geometry = 'x<1.0001'
    included_subdomain_ids = 1
    new_sideset_name = 'injection_area'
    input = 'aquifer'
  []
  [rename]
    type = RenameBlockGenerator
    old_block = '0 1'
    new_block = 'caps aquifer'
    input = 'injection_area'
  []
[]

[Variables]
  [dummy_var]
  []
[]
[Kernels]
  [dummy_diffusion]
    type = Diffusion
    variable = dummy_var
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  file_base = 2D_mesh
  exodus = true
[]
