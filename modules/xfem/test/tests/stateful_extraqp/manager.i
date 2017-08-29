[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[UserObjects]
  [./manager]
    type = XFEMMaterialManager
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[DiracKernels]
  [./managed_source]
    type = ManagedSourceTest
    variable = u
  [../]
[]

[Kernels]
  [./dt]
    type = TimeDerivative
    variable = u
  [../]
[]

[Executioner]
  type = Transient
[]
