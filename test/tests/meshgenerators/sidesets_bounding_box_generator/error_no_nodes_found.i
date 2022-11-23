[Mesh]
  [./gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    #parallel_type = replicated
  []

  [./createNewSidesetOne]
    type = SideSetsFromBoundingBoxGenerator
    input = gmg
    boundaries_old = 'right'
    boundary_new = 11
    bottom_left = '0.5 0.5 0'
    top_right = '1.9 1.9 0'
    block_id = 0
  []
  [./createNewSidesetTwo]
    type = SideSetsFromBoundingBoxGenerator
    input = createNewSidesetOne
    boundaries_old = 'top right'
    boundary_new = 10
    bottom_left = '-0.1 -0.1 0'
    top_right = '0.7 0.3 0'
    block_id = 0
    boundary_id_overlap = true
  []
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./leftBC]
    type = DirichletBC
    variable = u
    boundary = 10
    value = 1
  [../]
  [./rightBC]
    type = DirichletBC
    variable = u
    boundary = 11
    value = 0
  [../]
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
