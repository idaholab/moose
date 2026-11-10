# Creates the mesh for the remainder of the tutorial
[Mesh]
  [annular]
    type = AnnularMeshGenerator
    nr = 10
    rmin = 1.0
    rmax = 10
    growth_r = 1.4
    nt = 4
    dmin = 0
    dmax = 90
  []
  [make3D]
    type = AdvancedExtruderGenerator
    direction = '0 0 1'
    heights = '4 4 4'
    num_layers = '1 1 1'
    bottom_boundary = 'bottom'
    top_boundary = 'top'
    input = annular
  []
  [shift_down]
    type = TransformGenerator
    transform = TRANSLATE
    vector_value = '0 0 -6'
    input = make3D
  []
  [aquifer]
    type = SubdomainBoundingBoxGenerator
    block_id = 1
    bottom_left = '0 0 -2'
    top_right = '10 10 2'
    input = shift_down
  []
  [injection_area]
    type = ParsedGenerateSideset
    combinatorial_geometry = 'x*x+y*y<1.01'
    included_subdomains = 1
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
  file_base = 3D_mesh
  exodus = true
[]
