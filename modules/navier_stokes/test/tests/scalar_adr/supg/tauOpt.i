velocity=1

[GlobalParams]
  u = ${velocity}
  p = 0
  tau_type = opt
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 15
  xmax = 15
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./adv]
    type = Advection
    variable = u
  [../]
  [./frc]
    type = BodyForce
    variable = u
    function = 'ffn'
  [../]
  [./adv_supg]
    type = AdvectionSUPG
    variable = u
  [../]
  [./body_force_supg]
    type = BodyForceSUPG
    variable = u
    function = 'ffn'
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
[]

[Materials]
  [./mat]
    type = GenericConstantMaterial
    prop_names = 'mu rho'
    prop_values = '0 1'
  [../]
[]

[Functions]
  [./ffn]
    type = ParsedFunction
    value = 'if(x < 6, 1 - .25 * x, if(x < 8, -2 + .25 * x, 0))'
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
