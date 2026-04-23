#
# Test calculation of normal curvatures of an interface defined by order parameter c
#
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 8
  ny = 8
  nz = 8
  xmin = 0
  xmax = 20
  ymin = -10
  ymax = 10
  zmin = 0
  zmax = 20
[]

[Modules]
  [PhaseField]
    [Conserved]
      [./c]
        solve_type = direct
        free_energy = F
        kappa = 2.0
        mobility = 1.0
      []
    []
  []
[]

[ICs]
  [InitialCondition]
    type = FunctionIC
    function = ic_func_c
    variable = c
  []
[]

[Functions]
  [ic_func_c]
    type = ParsedFunction
    symbol_names = ' A   lambda '
    symbol_values = '0.5 40     '
    expression = '0.5*(1.0-tanh((y - A * sin(2*pi*x/lambda) * sin(2*pi*z/lambda)) / 2))'
  []
[]

[Materials]
  [free_energy]
    type = DerivativeParsedMaterial
    property_name = F
    coupled_variables = 'c'
    expression = 'c^2 * (1 - c)^2'
  []
  [curvatures]
    type = InterfaceNormalCurvatures
    eta = c
    outputs = 'exodus'
  []
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  solve_type = 'NEWTON'

  l_max_its = 30
  l_tol = 1.0e-4
  nl_max_its = 10
  nl_rel_tol = 1.0e-9
  nl_abs_tol = 1.0e-11

  start_time = 0.0
  end_time = 0.001
  dt = 0.001
[]

[Outputs]
  exodus = true
[]
