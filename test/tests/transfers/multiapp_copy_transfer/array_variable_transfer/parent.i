[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  # Array variable with two components
  [v]
    order = FIRST
    family = LAGRANGE
    components = 2
  []
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
  # Transfers all components together on the same mesh.
  [./from_sub]
    type = MultiAppCopyTransfer
    source_variable = u
    variable = v
    from_multi_app = sub
  [../]
[]

[Outputs]
  exodus = true
[]
