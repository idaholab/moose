# 2D, removal of a block containing a nodeset inside it
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
  [./SubdomainBoundingBox1]
    type = SubdomainBoundingBox
    block_id = 1
    bottom_left = '0 0 0'
    top_right = '4 4 1'
  [../]
  [./interior_nodeset]
    type = BoundingBoxNodeSet
    new_boundary = interior_ns
    bottom_left = '2 2 0'
    top_right = '3 3 1'
  [../]
  [./ed0]
    type = BlockDeleter
    block_id = 1
    depends_on = 'SubdomainBoundingBox1 interior_nodeset'
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
