#
# Test the split parsed function free enery Cahn-Hilliard Bulk kernel
# with two concentration variables and coupling through off-diagonal Onsager
# matrix coefficients
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmin = 0
  xmax = 60
  ymin = 0
  ymax = 60
  elem_type = QUAD4
[]

[Variables]
  [./c1]
    [./InitialCondition]
      type = FunctionIC
      function = 'cos(x/60*pi)'
    [../]
  [../]
  [./c2]
    [./InitialCondition]
      type = FunctionIC
      function = 'cos(y/60*pi)'
    [../]
  [../]
  [./w1]
  [../]
  [./w2]
  [../]
[]

[Kernels]
  [./c1_res]
    type = SplitCHParsed
    variable = c1
    f_name = F
    kappa_name = kappa_c
    w = w1
  [../]
  [./w11_res]
    type = SplitCHWRes
    variable = w1
    mob_name = M11
  [../]
  [./w12_res]
    type = SplitCHWRes
    variable = w1
    w = w2
    mob_name = M12
  [../]

  [./c2_res]
    type = SplitCHParsed
    variable = c2
    f_name = F
    kappa_name = kappa_c
    w = w2
  [../]
  [./w22_res]
    type = SplitCHWRes
    variable = w2
    mob_name = M22
  [../]
  [./w21_res]
    type = SplitCHWRes
    variable = w2
    w = w1
    mob_name = M21
  [../]

  [./time1]
    type = CoupledTimeDerivative
    variable = w1
    v = c1
  [../]
  [./time2]
    type = CoupledTimeDerivative
    variable = w2
    v = c2
  [../]
[]

[Materials]
  [./pfmobility]
    type = GenericConstantMaterial
    prop_names  = 'M11 M12 M21 M22 kappa_c'
    prop_values = '10  2.5 20  5   40'
  [../]

  [./free_energy]
    # equivalent to `MathFreeEnergy`
    type = DerivativeParsedMaterial
    property_name = F
    coupled_variables = 'c1 c2'
    expression = '0.25*(1+c1)^2*(1-c1)^2 + 0.25*(1+c2)^2*(1-c2)^2'
    derivative_order = 2
  [../]
[]

[Preconditioning]
  # active = ' '
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2

  solve_type = 'NEWTON'
  petsc_options_iname = -pc_type
  petsc_options_value = lu

  l_max_its = 30
  l_tol = 1.0e-4
  nl_rel_tol = 1.0e-10
  start_time = 0.0
  num_steps = 2

  dt = 10
[]

[Outputs]
  exodus = true
[]
