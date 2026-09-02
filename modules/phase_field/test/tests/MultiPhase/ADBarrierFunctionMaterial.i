# Derived from (modules/phase_field/test/tests/MultiPhase/barrierfunctionmaterial.i)

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
  xmin = 0
  xmax = 20
  ymin = 0
  ymax = 20
  elem_type = QUAD4
[]

[Variables]
  [eta]
  []
[]

[ICs]
  [IC_eta]
    type = SmoothCircleIC
    variable = eta
    x1 = 10
    y1 = 10
    radius = 5
    invalue = 1
    outvalue = 0
    int_width = 1
  []
[]

[Kernels]
  [eta_bulk]
    type = ADAllenCahn
    variable = eta
    f_name = F
    mob_name = L
  []
  [eta_interface]
    type = ADACInterface
    variable = eta
    kappa_name = K
    mob_name = F
    variable_L = false
  []

  [detadt]
    type = ADTimeDerivative
    variable = eta
  []
[]

[Materials]
  [barrier]
    type = ADBarrierFunctionMaterial
    eta = eta
    g_order = HIGH
    outputs = exodus
  []
  [const]
    type = ADGenericConstantMaterial
    prop_names = 'L K'
    prop_values = '1 1'
  []
  [F]
    type = ADDerivativeParsedMaterial
    property_name = F
    coupled_variables = eta
    expression = 'eta'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  num_steps = 1
[]

[Problem]
  solve = false
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
