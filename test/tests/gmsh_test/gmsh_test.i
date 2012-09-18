[Mesh]
  type = MooseMesh
  file = /Users/gastdr/projects/herd_trunk/moose_test/tests/gmsh_test/sample.msh
  dim = 3
  [./ExtraNodesets]
    [./left]
      nodes = 4
      id = 0
    [../]
    [./right]
      nodes = 9
      id = 1
    [../]
  [../]
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
    boundary = 0
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  [../]
[]

[Executioner]
  type = Steady
  petsc_options = -snes_mf_operator
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Output]
  output_initial = true
  exodus = true
  perf_log = true
[]

