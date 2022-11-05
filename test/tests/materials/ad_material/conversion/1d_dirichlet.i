[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmax = 2
[]

[Variables]
  [v]
    initial_condition = 1.1
  []
[]

[Kernels]
  inactive = 'ad_diff'
  [diff]
    type = MatDiffusion
    variable = v
    diffusivity = 'coef'
  []
  [ad_diff]
    type = ADMatDiffusion
    variable = v
    diffusivity = 'ad_coef_2'
  []
  [sink]
    type = ADBodyForce
    variable = v
    function = 'sink'
  []
[]

[BCs]
  [bounds]
    type = DirichletBC
    variable = v
    boundary = 'left right'
    value = 0
  []
[]

[Functions]
  [sink]
    type = ParsedFunction
    expression = '3*x^3'
  []
[]

[Materials]
  [ad_coef]
    type = ADParsedMaterial
    property_name = 'ad_coef'
    expression = '0.01 * max(v, 1)'
    coupled_variables = 'v'
  []
  [converter_to_regular]
    type = MaterialADConverter
    ad_props_in = 'ad_coef'
    reg_props_out = 'coef'
  []
  # at this point we should have lost the derivatives
  [converter_to_ad]
    type = MaterialADConverter
    reg_props_in = 'coef'
    ad_props_out = 'ad_coef_2'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]
