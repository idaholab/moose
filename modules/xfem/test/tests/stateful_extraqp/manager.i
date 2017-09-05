[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
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
    type = StatefulTestMaterial
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
  dt = 0.1
  num_steps = 3
[]
