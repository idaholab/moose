# This test is here to verify that we can delete a Multiapp
# Early after initialSetup. In order for a multiapp to be
# deleted early it must be set to execute_on = initial ONLY.

[Mesh]
  type = GeneratedMesh
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
  [./time]
    type = TimeDerivative
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

[Executioner]
  type = Transient
  dt = 1
  num_steps = 3

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[MultiApps]
  [./early_delete]
    type = EarlyDeleteMultiApp

    # Must be initial ONLY!
    execute_on = initial
    positions = '0 0 0'
    input_files = sub.i
  [../]
[]
