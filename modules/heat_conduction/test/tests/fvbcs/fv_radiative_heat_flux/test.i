[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
    xmax = 2
  []
[]

[Variables]
  [u]
    type = MooseVariableFVReal
    initial_condition = 0.5
  []
[]

[FVKernels]
  [diff_left]
    type = FVDiffusion
    variable = u
    coeff = 4
  []
  [gradient_creating]
    type = FVBodyForce
    variable = u
  []
[]

[FVBCs]
  [left]
    type = FVInfiniteCylinderRadiativeBC
    variable = u
    boundary = 'left'
    boundary_radius = 1
    cylinder_radius = 12
    cylinder_emissivity = 0.4
  []
  [top]
    type = FVInfiniteCylinderRadiativeBC
    variable = u
    # Test setting it separately
    temperature = 'u'
    boundary = 'top'
    boundary_radius = 1
    cylinder_radius = 12
    cylinder_emissivity = 0.4
  []
  [other]
    type = FVDirichletBC
    variable = u
    boundary = 'right bottom'
    value = 0
  []
[]

[Materials]
  [cht]
    type = ADGenericConstantMaterial
    prop_names = 'htc'
    prop_values = '1'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
