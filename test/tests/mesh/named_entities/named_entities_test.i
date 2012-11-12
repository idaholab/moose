[Mesh]
  file = named_entities.e
  uniform_refine = 1
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
    block = '1 center_block 3'

    [./InitialCondition]
      type = ConstantIC
      value = 20
      block = 'center_block 3'
    [../]
  [../]
[]

[AuxVariables]
  [./reporter]
    order = CONSTANT
    family = MONOMIAL
    block = 'left_block 3'
  [../]
[]

[Kernels]
  active = 'diff body_force'

  [./diff]
    type = Diffusion
    variable = u
    # Note we are using both names and numbers here
    block = 'left_block 2 right_block'
  [../]

  [./body_force]
    type = BodyForce
    variable = u
    block = 'center_block'
    value = 10
  [../]
[]

[AuxKernels]
  [./hardness]
    type = MaterialRealAux
    variable = reporter
    property = 'hardness'
    block = 'left_block 3'
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 'left_side'
    value = 1
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 'right_side'
    value = 1
  [../]
[]

[Postprocessors]
  [./elem_average]
    type = ElementAverageValue
    variable = u
    block = 'center_block'
  [../]

  [./side_average]
    type = SideAverageValue
    variable = u
    boundary = 'right_side'
  [../]
[]

[Materials]
  [./constant]
    type = GenericConstantMaterial
    prop_names = 'hardness'
    prop_values = 10
    block = '1 right_block'
  [../]

  [./empty]
    type = MTMaterial
    block = 'center_block'
  [../]
[]

[Executioner]
  type = Steady
  petsc_options = '-snes_mf_operator'
[]

[Output]
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
[]
