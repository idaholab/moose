[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
  xmax = 0.7
  ymax = 0.7
[]
[Variables]
  [w]
  []
  [T]
  []
[]
[ICs]
  [wIC]
    type = SmoothCircleIC
    variable = w
    int_width = 0.3
    x1 = 0.35
    y1 = 0.35
    radius = 0.2
    outvalue = 0.1
    invalue = 0.9
  []
  [TIC]
    type = FunctionIC
    variable = T
    function = 'x+y'
  []
[]
[Kernels]
  [w_dot]
    type = TimeDerivative
    variable = w
  []
  [k1]
    type = ACInterfaceKobayashi1
    variable = w
    mob_name = M
    coupled_variables = T
  []
  [k2]
    type = ACInterfaceKobayashi2
    variable = w
    mob_name = M
    coupled_variables = T
  []
  [T_dot]
    type = TimeDerivative
    variable = T
  []
[]
[Materials]
  [material]
    type = InterfaceOrientationMaterial
    op = w
  []
  [M]
    type = DerivativeParsedMaterial
    property_name = M
    coupled_variables = 'w T'
    expression = '1 + w^2 + T^2'
  []
[]
[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]
[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = 0.001
  num_steps = 1
[]
