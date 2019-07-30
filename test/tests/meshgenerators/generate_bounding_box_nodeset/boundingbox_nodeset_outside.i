[MeshGenerators]
  [./gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
    #parallel_type = replicated
  []

  [./nodeset]
    type = BoundingBoxNodeSetGenerator
    input = gmg
    new_boundary = middle_node
    top_right = '1.1 1.1 0'
    bottom_left = '0.51 0.51 0'
    location = OUTSIDE
  []
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
  [./middle]
    type = DirichletBC
    variable = u
    boundary = middle_node
    value = -1
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
[]

[Outputs]
  exodus = true
[]
