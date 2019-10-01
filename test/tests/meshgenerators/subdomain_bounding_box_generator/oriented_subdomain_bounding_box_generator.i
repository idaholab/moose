[Mesh]
  [./gmg]
    type = GeneratedMeshGenerator
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

  [./subdomains]
    type = OrientedSubdomainBoundingBoxGenerator
    input = gmg
    center = '-1 4 1'
    width = 5
    length = 10
    height = 4
    width_direction = '2 1 0'
    length_direction = '-1 2 2'
    block_id = 10
  []
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

[Executioner]
  type = Steady
  solve_type = PJFNK
[]

[Outputs]
  exodus = true
[]
