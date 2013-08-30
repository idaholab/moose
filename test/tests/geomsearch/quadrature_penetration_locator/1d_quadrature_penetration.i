[Mesh]
  type = FileMesh
  file = 1d_penetration.e
  construct_side_list_from_node_list = true
  # This test will not work in parallel with ParallelMesh enabled
  # due to a bug in the GeometricSearch system.
  distribution = serial
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./penetration]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[AuxBCs]
  [./penetration]
    type = PenetrationAux
    variable = penetration
    boundary = 2
    paired_boundary = 3
  [../]
[]

[BCs]
  [./block1_left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]
  [./block1_right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
  [./block2_left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  [../]
  [./block2_right]
    type = DirichletBC
    variable = u
    boundary = 4
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

[]

[Output]
  output_initial = true
  exodus = true
  perf_log = true
[]

