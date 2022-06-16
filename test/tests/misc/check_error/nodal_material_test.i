[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 4
  ny = 4
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

# Nodal Auxvariable that tries to access a material property
[AuxVariables]
active = 'mat'
  [./mat]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff'

  [./diff]
    type = MatDiffusionTest
    variable = u
    prop_name = matp
  [../]
[]

[AuxKernels]
active = 'mat'
  [./mat]
    type = MaterialRealAux
    variable = mat
    property = matp
  [../]
[]

[BCs]
  active = 'left right'

  [./left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 1
  [../]

  [./right]
    type = MTBC
    variable = u
    boundary = 1
    grad = 8
    prop_name = matp
  [../]
[]

[Materials]
  active = mat

  [./mat]
    type = MTMaterial
    block = 0
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  file_base = out
[]
