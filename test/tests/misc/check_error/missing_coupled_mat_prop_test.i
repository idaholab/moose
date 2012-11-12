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
  # This material is global and uses a coupled property
  [./mat_global]
    type = CoupledMaterial
    mat_prop = 'some_prop'
    coupled_mat_prop = 'mp1'
    block = '1 2'
  [../]

  # This material supplies a value for block 1 ONLY
  [./mat_0]
    type = GenericConstantMaterial
    block = 1
    prop_names = 'mp1'
    prop_values = 2
  [../]
[]

[Executioner]
  type = Steady
#  solve_type = 'PJFNK'
#  preconditioner = 'ILU'
  petsc_options = '-snes_mf_operator'
#  petsc_options_iname = '-pc_type -pc_hypre_type'
#  petsc_options_value = 'hypre boomeramg'
[]

[Output]
  file_base = missing_mat_prop_test
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
[]


