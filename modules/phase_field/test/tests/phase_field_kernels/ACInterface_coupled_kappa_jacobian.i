[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
  xmax = 4
  ymax = 4
[]
[Variables]
  [eta]
    [InitialCondition]
      type = FunctionIC
      function = '0.2+0.1*x+0.05*y*y'
    []
  []
  [chi]
    [InitialCondition]
      type = FunctionIC
      function = '0.5+0.1*x*y'
    []
  []
[]
[Kernels]
  [detadt]
    type = TimeDerivative
    variable = eta
  []
  [ACInterface]
    type = ACInterface
    variable = eta
    kappa_name = kappa
    mob_name = L
    variable_L = true
    coupled_variables = chi
  []
  [dchidt]
    type = TimeDerivative
    variable = chi
  []
[]
[Materials]
  [L]
    type = DerivativeParsedMaterial
    property_name = L
    coupled_variables = 'eta chi'
    expression = '0.1 * eta^2 + chi^2 + eta*chi'
    derivative_order = 2
  []
  [kappa]
    type = DerivativeParsedMaterial
    property_name = kappa
    coupled_variables = 'eta chi'
    expression = '1 + eta^2 + 2*chi^2'
    derivative_order = 1
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
