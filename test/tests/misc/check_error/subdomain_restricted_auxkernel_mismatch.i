[Mesh]
  file = rectangle.e
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./foo]
    order = FIRST
    family = LAGRANGE
    block = 1
  [../]
[]

[Kernels]
  active = 'diff body_force'

  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./body_force]
    type = BodyForce
    variable = u
    block = 1
    value = 10
  [../]
[]

[AuxKernels]
  [./foo]
    type = ConstantAux
    variable = foo
    value = 1
    block = 2
  [../]
[]

[BCs]
  active = 'right'

  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
#  petsc_options = '-snes_mf_operator'
#  petsc_options_iname = '-pc_type -pc_hypre_type'
#  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  file_base = out
[]
