[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[UserObjects]
  [./points]
    type = ExtraQPTest
    points = '0.79 0.13 0
              0.39 0.42 0'
  [../]
  [./manager]
    type = XFEMMaterialManager
    extra_qps = points
    material_names = 'material1 material2'
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[DiracKernels]
  [./managed_source]
    type = ManagedSourceTest
    manager = manager
    variable = u
  [../]
[]

[Materials]
  [./material1]
    type = GenericConstantMaterial
    prop_names  = 'prop1 prop2'
    prop_values = '1     2'
    compute = false
  [../]
  [./material2]
    type = GenericConstantMaterial
    prop_names  = 'prop3 prop4'
    prop_values = '3     4'
    compute = false
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
