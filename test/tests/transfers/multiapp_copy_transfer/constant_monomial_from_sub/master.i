[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
    family = MONOMIAL
    order = CONSTANT
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
  [./from_sub]
    type = MultiAppCopyTransfer
    source_variable = aux
    variable = u
    from_multi_app = sub
  [../]
[]

[Outputs]
  exodus = true
[]
