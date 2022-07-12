[Mesh]
  file = three_block.e

  # These names will be applied on the fly to the
  # mesh so they can be used in the input file
  # In addition they will show up in the input file
  block_id = '1 2 3'
  block_name = 'wood steel copper'

  boundary_id = '1 2'
  boundary_name = 'left left'      # Can't have duplicate names
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff'

  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  active = 'left right'

  [./left]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 'right'
    value = 1
  [../]
[]

[Materials]
  active = empty

  [./empty]
    type = MTMaterial
    block = 'wood steel copper'
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]
