[Mesh]
  file = 2-lines.e
[]

[Variables]
  [./u]
    family = LAGRANGE
    order = FIRST
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
    boundary = 1
    value = 1
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 4
    value = 3
  [../]
[]

[Constraints]
  [./c1]
    type = EqualValueNodalConstraint
    variable = u
    master = 0
    slave = 4
    penalty = 100000
  [../]
[]

[Executioner]
  type = Steady
  petsc_options = '-snes_mf_operator'
[]

[Output]
  output_initial = true
  exodus = true
[]