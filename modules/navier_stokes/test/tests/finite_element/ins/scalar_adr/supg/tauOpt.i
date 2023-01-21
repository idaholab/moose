velocity=1

[GlobalParams]
  u = ${velocity}
  pressure = 0
  tau_type = opt
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 15
  xmax = 15
[]

[Variables]
  [./c]
  [../]
[]

[Kernels]
  [./adv]
    type = Advection
    variable = c
    forcing_func = 'ffn'
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = c
    boundary = left
    value = 0
  [../]
[]

[Materials]
  [./mat]
    # These Materials are required by the INSBase class; we don't use them for anything.
    type = GenericConstantMaterial
    prop_names = 'mu rho'
    prop_values = '0 1'
  [../]
[]

[Functions]
  [./ffn]
    type = ParsedFunction
    expression = 'if(x < 6, 1 - .25 * x, if(x < 8, -2 + .25 * x, 0))'
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
