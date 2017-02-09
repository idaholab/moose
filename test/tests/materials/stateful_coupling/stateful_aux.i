[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./aux]
    order = FIRST
    family = LAGRANGE
    initial_condition = 2
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
  # This material couples to an aux variable and
  # uses it in stateful property initialization
  [./stateful_mat]
    type = StatefulTest
    coupled = aux
    prop_names = thermal_conductivity
    prop_values = -1 # ignored
    output_properties = thermal_conductivity
    outputs = exodus
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 4
[]

[Outputs]
  exodus = true
[]

[Debug]
  show_material_props = true
[]
