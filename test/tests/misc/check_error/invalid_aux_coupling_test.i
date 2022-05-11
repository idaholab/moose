[Mesh]
  [./square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  [../]
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./nodal_aux]
    order = FIRST
    family = LAGRANGE
  [../]

  [./elemental_aux]
#    order = FIRST
#    family = LAGRANGE
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  active = 'diff'

  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[AuxKernels]
  [./nodal]
    type = CoupledAux
    variable = nodal_aux
    coupled = elemental_aux
  [../]

  [./elemental]
    type = ConstantAux
    variable = elemental_aux
    value = 6
  [../]
[]

[BCs]
  active = 'left right'

  [./left]
    type = DirichletBC
    variable = u
    boundary = 3
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

  solve_type = 'PJFNK'
[]

[Outputs]
  file_base = out
[]
