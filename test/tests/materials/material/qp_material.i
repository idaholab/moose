[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 3
  elem_type = QUAD4
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left_u]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]

  [./right_u]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Materials]
  [./mat1]
    type = QpMaterial
    block = 0
    outputs = all
    constant_on = ELEMENT
    property_name = 'zero_prop'
  [../]
  # The second copy of QpMaterial is not constant_on_elem.
  [./mat2]
    type = QpMaterial
    block = 0
    outputs = all
    property_name = 'nonzero_prop'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]
