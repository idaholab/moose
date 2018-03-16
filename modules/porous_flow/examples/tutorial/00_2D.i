# Creates the mesh for the remainder of the tutorial
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  xmin = 1.0
  xmax = 10
  bias_x = 1.4
  ny = 3
  ymin = -6
  ymax = 6
[]

[MeshModifiers]
  [./aquifer]
    type = SubdomainBoundingBox
    block_id = 1
    bottom_left = '0 -2 0'
    top_right = '10 2 0'
  [../]
  [./injection_area]
    type = ParsedAddSideset
    combinatorial_geometry = 'x<1.0001'
    included_subdomain_ids = 1
    new_sideset_name = 'injection_area'
    depends_on = 'aquifer'
  [../]
  [./rename]
    type = RenameBlock
    old_block_id = '0 1'
    new_block_name = 'caps aquifer'
    depends_on = 'injection_area'
  [../]
[]

[Variables]
  [./dummy_var]
  [../]
[]
[Kernels]
  [./dummy_diffusion]
    type = Diffusion
    variable = dummy_var
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  file_base = 2D_mesh
  exodus = true
[]

