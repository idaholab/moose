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
    type = FunctorThermalResistanceBC
    geometry = 'cartesian'
    variable = u
    T_ambient = 10
    htc = 'htc'
    emissivity = 0.2
    thermal_conductivities = '0.1 0.2 0.3'
    conduction_thicknesses = '1 0.7 0.2'
    boundary = 'left'

    # Test setting iteration parameters
    step_size = 0.02
    max_iterations = 120
    tolerance = 1e-4
  []
  [top]
    type = FunctorThermalResistanceBC
    geometry = 'cartesian'
    variable = u
    # Test setting the temperature separately from the variable
    temperature = 'u'
    T_ambient = 14
    htc = 'htc'
    emissivity = 0
    thermal_conductivities = '0.1 0.2 0.3'
    conduction_thicknesses = '1 0.7 0.4'
    boundary = 'top'
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
    type = ADGenericFunctorMaterial
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
