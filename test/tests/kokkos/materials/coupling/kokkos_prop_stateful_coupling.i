[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  active = 'u'

  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [diff]
    type = KokkosMatDiffusionTest
    variable = u
    prop_name = 'some_prop'
  []

  [time]
    type = KokkosTimeDerivative
    variable = u
  []
[]

[BCs]
  [left]
    type = KokkosDirichletBC
    variable = u
    boundary = 'left'
    value = 1
  []

  [right]
    type = KokkosDirichletBC
    variable = u
    boundary = 'right'
    value = 2
  []
[]

[Materials]
  # This material couples in a stateful property from StatefulTest
  [coupled_mat]
    type = KokkosCoupledMaterial
    mat_prop = 'some_prop'
    coupled_mat_prop = 'thermal_conductivity'
    use_old_prop = true
  []

  [stateful_mat]
    type = KokkosStatefulTest
    prop_names = thermal_conductivity
    prop_values = 1.0
  []
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
