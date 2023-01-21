[Mesh]
  type = GeneratedMesh
  dim = 2
  xmax = 10
  ymax = 10
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./phi]
  [../]
[]

[ICs]
  [./phi_IC]
    type = FunctionIC
    variable = phi
    function = ic_func_phi
  [../]
[]

[Functions]
  [./ic_func_phi]
    type = ParsedFunction
    expression = '0.5 * (1 - tanh((x - 5) / 0.8))'
  [../]
[]

[BCs]
  [./top]
    type = ADMatNeumannBC
    variable = u
    boundary = top
    value = 2
    boundary_material = hm
  [../]
[]

[Kernels]
  [./dudt]
    type = ADTimeDerivative
    variable = u
  [../]
  [./diff]
    type = ADDiffusion
    variable = u
  [../]
[]

[Materials]
  [./hm]
    type = ADParsedMaterial
    property_name = hm
    coupled_variables = 'phi'
    expression = '3*phi^2 - 2*phi^3'
    outputs = exodus
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'
  end_time = 10
[]

[Outputs]
  exodus = true
[]
