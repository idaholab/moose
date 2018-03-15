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

[Materials]
  [./mat_3]
    type = GenericConstantMaterial
    prop_names = 'prop3'
    prop_values = '300'
    block = '1 2'
  [../]

  [./mat_2]
    type = CoupledMaterial
    mat_prop = 'prop2'
    coupled_mat_prop = 'prop3'
    block = '1 2'
  [../]

  [./mat_1]
    type = CoupledMaterial2
    mat_prop = 'prop1'
    coupled_mat_prop1 = 'prop2'
    coupled_mat_prop2 = 'prop3'
    block = '1 2'
  [../]
[]

[Executioner]
  type = Steady
#  solve_type = 'PJFNK'
#  preconditioner = 'ILU'

  solve_type = 'PJFNK'
#  petsc_options_iname = '-pc_type -pc_hypre_type'
#  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  file_base = out_adv_coupled2
  exodus = true
[]
