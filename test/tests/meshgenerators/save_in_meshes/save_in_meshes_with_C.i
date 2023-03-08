[Mesh]
  [A]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 3
    ny = 3
    xmin = 3
    xmax = 6
    ymin = -5
    ymax = 5
  []

  [B]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 3
    ny = 3
    xmin = -3
    xmax = 0
    ymin = -5
    ymax = 5
  []

  [C]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 3
    xmin = -3
    xmax = 0
    save_with_name = 'C'
  []

  [A_and_B]
    type = MeshCollectionGenerator
    inputs = 'A B'
  []

  final_generator = 'A_and_B'
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [test]
    type = TestSaveInMesh
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 1
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 0
  []
[]

[Problem]
  type = FEProblem
  allow_invalid_solution = false
  immediately_print_invalid_solution = false
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu superlu_dist'
[]

[Outputs]
 exodus = true
[]
