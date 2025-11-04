[Physics]
  [Test]
    [ComponentInterfaceErrors]
      [added_from_component]
      []
    []
  []
[]

[Variables]
  [test_variable]
  []
[]

[ActionComponents]
  [cylinder_2]
    type = CylinderComponent
    dimension = 2
    radius = 4
    length = 1
    n_axial = 1
    n_radial = 1
    position = '2 0 0'
    direction = '0 0 1'
    physics = 'added_from_component'
    block = 'cyl2'
    verbose = true
  []
[]

[Executioner]
  type = Steady
[]
