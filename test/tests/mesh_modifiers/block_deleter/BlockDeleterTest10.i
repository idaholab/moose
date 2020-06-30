# 2D, removal of a block containing a sideset inside it
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
  xmin = 0
  xmax = 5
  ymin = 0
  ymax = 5
[]

[MeshModifiers]
  [./left]
    type = SubdomainBoundingBox
    block_id = 1
    bottom_left = '2 2 0'
    top_right = '3 3 1'
  [../]
  [./right]
    type = SubdomainBoundingBox
    block_id = 2
    bottom_left = '3 2 0'
    top_right = '4 3 1'
  [../]
  [./interior_sideset]
    type = SideSetsBetweenSubdomains
    primary_block = 1
    paired_block = 2
    depends_on = 'left right'
    new_boundary = interior_ss
  [../]
  [./new_block_number]
    type = SubdomainBoundingBox
    block_id = 3
    bottom_left = '0 0 0'
    top_right = '4 4 1'
    depends_on = 'interior_sideset'
  [../]
  [./ed0]
    type = BlockDeleter
    block_id = 3
    depends_on = 'new_block_number'
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./dt]
    type = TimeDerivative
    variable = u
  [../]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./top]
    type = DirichletBC
    variable = u
    boundary = bottom
    value = 1
  [../]
[]

[Executioner]
  type = Transient
  start_time = 0
  end_time = 10
  dt = 10

  solve_type = NEWTON

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
