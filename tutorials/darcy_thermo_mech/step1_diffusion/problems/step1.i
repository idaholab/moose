[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 100
  ny = 10
  xmax = 0.304  # Length of test chamber
  ymax = 0.0257  # Half inner diameter of test chamber
[]

[Variables]
  [./pressure]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = pressure
  [../]
[]

[BCs]
  [./inlet]
    type = DirichletBC
    variable = pressure
    boundary = left
    value = 30 # Chosen to give a delta_P/L of ~ 100 kPa/m
  [../]
  [./outlet]
    type = DirichletBC
    variable = pressure
    boundary = right
    value = 0 # Chosen to give a delta_P/L of ~ 100 kPa/m
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  output_initial = true
  exodus = true
  [./console]
    type = Console
    perf_log = true
    linear_residuals = true
  [../]
[]
