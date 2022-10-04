[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 10
    ny = 10
    nz = 10
  []

  [createNewSidesetOne]
    type = SideSetsFromBoundingBoxGenerator
    input = gen
    block_id = 0
    boundary_id_old = 'bottom back left'
    boundary_new = 10
    bottom_left = '-1.1 -1.1 -1.1'
    top_right = '1.1 1.1 1.1'
    boundary_id_overlap = true
  []
  [createNewSidesetTwo]
    type = SideSetsFromBoundingBoxGenerator
    input = createNewSidesetOne
    block_id = 0
    boundary_id_old = 'right bottom'
    boundary_new = 11
    bottom_left = '-1.1 -1.1 -1.1'
    top_right = '1.1 1.1 1.1'
    boundary_id_overlap = true
  []
  [createNewSidesetThree]
    type = SideSetsFromBoundingBoxGenerator
    input = createNewSidesetTwo
    block_id = 0
    boundary_id_old = 'top front'
    boundary_new = 12
    bottom_left = '-1.1 -1.1 -1.1'
    top_right = '1.1 1.1 1.1'
    boundary_id_overlap = true
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [BCone]
    type = DirichletBC
    variable = u
    boundary = 10
    value = 1
  []
  [BCtwo]
    type = DirichletBC
    variable = u
    boundary = 11
    value = 1
  []
  [BCthree]
    type = DirichletBC
    variable = u
    boundary = 12
    value = 0
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
