#
# Test calculation of normal curvatures of an interface defined by order parameter c
#
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 8
  ny = 8
  nz = 8
  xmin = 8
  xmax = 12
  ymin = -2
  ymax = 2
  zmin = 8
  zmax = 12
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

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
