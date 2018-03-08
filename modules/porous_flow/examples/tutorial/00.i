# Creates the mesh for the remainder of the tutorial
[Mesh]
  type = AnnularMesh
  dim = 2
  nr = 10
  rmin = 1.0
  rmax = 10
  growth_r = 1.4
  nt = 4
  tmin = 0
  tmax = 1.57079632679
[]

[MeshModifiers]
  [./make3D]
    type = MeshExtruder
    extrusion_vector = '0 0 12'
    num_layers = 3
    bottom_sideset = 'bottom'
    top_sideset = 'top'
  [../]
  [./shift_down]
    type = Transform
    transform = TRANSLATE
    vector_value = '0 0 -6'
    depends_on = make3D
  [../]
  [./aquifer]
    type = SubdomainBoundingBox
    block_id = 1
    bottom_left = '0 0 -2'
    top_right = '10 10 2'
    depends_on = shift_down
  [../]
  [./injection_area]
    type = ParsedAddSideset
    combinatorial_geometry = 'x*x+y*y<1.01'
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
  file_base = 3D_mesh
  exodus = true
[]

