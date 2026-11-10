[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 3
  xmax = 3
  ymax = 3
[]
[Variables]
  [c]
    family = HERMITE
    order = THIRD
    [InitialCondition]
      type = FunctionIC
      function = '0.2+0.1*sin(x)*cos(y)'
    []
  []
  [d]
    [InitialCondition]
      type = FunctionIC
      function = '0.5+0.1*x+0.2*y'
    []
  []
  [e]
    [InitialCondition]
      type = FunctionIC
      function = '0.3+0.2*x*y'
    []
  []
[]
[Kernels]
  [c_int]
    type = CHInterface
    variable = c
    kappa_name = kappa_c
    mob_name = M
    coupled_variables = 'd e'
  []
  [c_dot]
    type = TimeDerivative
    variable = c
  []
  [d_dot]
    type = TimeDerivative
    variable = d
  []
  [e_dot]
    type = TimeDerivative
    variable = e
  []
[]
[Materials]
  [kappa]
    type = GenericConstantMaterial
    prop_names = kappa_c
    prop_values = 2.0
  []
  [mob]
    type = DerivativeParsedMaterial
    property_name = M
    coupled_variables = 'c d e'
    expression = '1 + c^2 + c*d + d^2*e + e^2'
    derivative_order = 3
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
  dt = 0.1
  num_steps = 1
[]
