[Mesh]
  type = GeneratedMesh
  dim = 3
  xmin = -6
  xmax = 4
  nx = 10
  ymin = -2
  ymax = 10
  ny = 12
  zmin = -5
  zmax = 7
  nz = 12
[]

[MeshModifiers]
  [./subdomains]
    type = OrientedSubdomainBoundingBox
    center = '-1 4 1'
    width = 5
    length = 10
    height = 4
    width_direction = '2 1 0'
    length_direction = '-1 2 2'
    block_id = 10
    location = INSIDE
  [../]
[]

[Problem]
  type = FEProblem
  solve = false
  kernel_coverage_check = false
[]

[Variables]
  [./u]
  [../]
[]

[Materials]
  [./mat10]
    type = GenericConstantMaterial
    block = 10
    outputs = all
    prop_values = 6.24
    prop_names = prop
  [../]
  [./mat0]
    type = GenericConstantMaterial
    block = 0
    prop_names = prop
    prop_values = 0
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
[]

[Outputs]
  exodus = true
[]
