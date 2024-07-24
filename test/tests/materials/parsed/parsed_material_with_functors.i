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
  [test_func]
    type = ParsedFunction
    expression = '1 + sin(y)'
  []
[]

[BCs]
  [top]
    type = MatNeumannBC
    variable = u
    boundary = top
    value = 2
    boundary_material = hm
  []
[]

[Kernels]
  [dudt]
    type = TimeDerivative
    variable = u
  []
  [diff]
    type = Diffusion
    variable = u
  []
[]

[Materials]
  [hm]
    type = ParsedMaterial
    property_name = hm
    coupled_variables = 'phi'
    functor_names = 'test_func'
    functor_symbols = 'tf'
    expression = '3*phi^2 - 2*phi^3 + tf'
    outputs = exodus
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
