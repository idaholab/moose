# Checking that coupling a constant monomial variable into an object that expects
# a nodal variable will report an error

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
  elem_type = EDGE2
[]

[Variables]
  [./v]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Materials]
  [./m]
    type = CoupledNodalMaterial
    coupled = v
  [../]
[]

[Executioner]
  type = Transient
[]
