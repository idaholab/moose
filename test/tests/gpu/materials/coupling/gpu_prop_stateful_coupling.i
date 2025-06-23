[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[GPUKernels]
  [./diff]
    type = GPUMatDiffusionTest
    variable = u
    prop_name = 'some_prop'
  [../]

  [./time]
    type = GPUTimeDerivative
    variable = u
  [../]
[]

[GPUBCs]
  [./left]
    type = GPUDirichletBC
    variable = u
    boundary = 'left'
    value = 1
  [../]

  [./right]
    type = GPUDirichletBC
    variable = u
    boundary = 'right'
    value = 2
  [../]
[]

[GPUMaterials]
  # This material couples in a stateful property from StatefulTest
  [./coupled_mat]
    type = GPUCoupledMaterial
    mat_prop = 'some_prop'
    coupled_mat_prop = 'thermal_conductivity'
    use_old_prop = true
  [../]

  [./stateful_mat]
    type = GPUStatefulTest
    prop_names = thermal_conductivity
    prop_values = 1.0
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  num_steps = 4
[]

[Outputs]
  exodus = true
[]

[Debug]
  show_material_props = true
[]
