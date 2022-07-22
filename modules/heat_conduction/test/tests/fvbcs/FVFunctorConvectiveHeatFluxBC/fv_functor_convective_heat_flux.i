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
  [T_fluid]
    type = MooseVariableFVReal
    initial_condition = 0
  []
[]

[AuxVariables]
  [T_wall]
    type = MooseVariableFVReal
    initial_condition = 1
  []
[]

[AuxKernels]
  [wall_temp]
    type = ConstantAux
    variable = T_wall
    value = 1
  []
[]

[FVKernels]
  [diff_left]
    type = FVDiffusion
    variable = T_fluid
    coeff = 4
  []
  [gradient_creating]
    type = FVBodyForce
    variable = T_fluid
  []
[]

[FVBCs]
  [top]
    type = FVFunctorConvectiveHeatFluxBC
    boundary = 'top'
    variable = T_fluid
    T_infinity = T_fluid
    T_wall = T_wall
    is_solid = false
    heat_transfer_coefficient = 'htc'
  []
  [other]
    type = FVDirichletBC
    variable = T_fluid
    boundary = 'bottom'
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
