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

[Kernels]
  [./diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
  [../]

  [./time]
    type = TimeDerivative
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 1
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 'right'
    value = 2
  [../]
[]

[Materials]
  # This material couples in a stateful property from StatefulTest
  [./coupled_mat]
    type = CoupledMaterial
    mat_prop = 'some_prop'
    coupled_mat_prop = 'thermal_conductivity'
    use_old_prop = true
  [../]

  [./stateful_mat]
    type = StatefulTest
    prop_names = thermal_conductivity
    prop_values = 1.0
    output_properties = thermal_conductivity
    outputs = exodus
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
