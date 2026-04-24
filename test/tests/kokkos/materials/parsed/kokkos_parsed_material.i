[Mesh]
  type = GeneratedMesh
  dim = 2
  xmax = 10
  ymax = 10
  nx = 10
  ny = 10
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [phi]
  []
  [hm]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[ICs]
  [phi_IC]
    type = FunctionIC
    variable = phi
    function = ic_func_phi
  []
[]

[Functions]
  [ic_func_phi]
    type = ParsedFunction
    expression = '0.5 * (1 - tanh((x - 5) / 0.8))'
  []
  [tf]
    type = KokkosParsedFunction
    expression = '1 + sin(y)'
  []
[]

[BCs]
  [top]
    type = KokkosMatNeumannBC
    variable = u
    boundary = top
    value = 2
    boundary_material = hm
  []
[]

[Kernels]
  [dudt]
    type = KokkosTimeDerivative
    variable = u
  []
  [diff]
    type = KokkosDiffusion
    variable = u
  []
[]

[AuxKernels]
  [hm]
    type = KokkosMaterialRealAux
    variable = hm
    property = hm
    execute_on = 'INITIAL'
  []
[]

[Materials]
  [mat]
    type = KokkosParsedMaterial
    property_name = hm
    coupled_variables = 'phi'
    function_names = 'tf'
    expression = '3*phi^2 - 2*phi^3 + tf'
  []
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'
  end_time = 10
[]

[Outputs]
  exodus = true
[]
