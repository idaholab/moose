[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  # Create two variables
  [./u]
  [../]
  [./v]
  [../]
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 1
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[MultiApps]
  [./sub]
    type = FullSolveMultiApp
    input_files = sub.i
    execute_on = initial
  [../]
[]

[Transfers]
  # Transfer both variables by inputting a vector of their names
  [./from_sub]
    type = MultiAppCopyTransfer
    source_variable = 'u v'
    variable = 'u v'
    from_multi_app = sub
  [../]
[]

[Outputs]
  exodus = true
[]
