[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmax = 0.2
  ymax = 0.2
[]

[Variables]
  [./sub_u]
  [../]
[]

[AuxVariables]
  [./transferred_u]
  [../]
  [./elemental_transferred_u]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = sub_u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = sub_u
    boundary = left
    value = 1
  [../]
  [./right]
    type = DirichletBC
    variable = sub_u
    boundary = right
    value = 4
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 1

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options = '-ksp_monitor'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Output]
  output_initial = true
  exodus = true
  perf_log = true
[]

