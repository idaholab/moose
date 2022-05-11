[Mesh]
  type = GeneratedMesh
  parallel_type = 'replicated'
  dim = 2
  nx = 10
  ny = 10
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
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[MultiApps]
  [./master]
    type = FullSolveMultiApp
    input_files = 'sub.i'
    execute_on = initial
  [../]
[]

[Transfers]
  [./transfer]
    type = MultiAppUserObjectTransfer
    to_multi_app = master
    user_object = data
    variable = aux
  [../]
[]

[VectorPostprocessors]
  [./data]
    type = CSVReader
    csv_file = 'example.csv'
  [../]
[]


[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]
