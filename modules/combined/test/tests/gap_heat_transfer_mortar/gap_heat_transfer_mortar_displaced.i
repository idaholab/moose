[Mesh]
  file = 2blk-gap.e
  displacements = 'disp_x disp_y'

  [./MortarInterfaces]
    [./middle]
      master = 100
      slave = 101
      subdomain = 1000
    [../]
  [../]
[]

[AuxVariables]
  [./disp_x]
    block = '1'
  [../]
  [./disp_y]
    block = '1'
  [../]
[]

[AuxKernels]
  [./disp_x_kernel]
    type = ConstantAux
    variable = disp_x
    value = 0.1
    block = '1'
  [../]

  [./disp_y_kernel]
    type = ConstantAux
    variable = disp_y
    value = 0
    block = '1'
  [../]
[]

[Variables]
  [./temp]
    order = FIRST
    family = LAGRANGE
    block = '1 2'
  [../]

  [./lm]
    order = FIRST
    family = LAGRANGE
    block = '1000'
  [../]
[]

[Materials]
  [./left]
    type = HeatConductionMaterial
    block = 1
    thermal_conductivity = 1000
    specific_heat = 1
  [../]

  [./right]
    type = HeatConductionMaterial
    block = 2
    thermal_conductivity = 500
    specific_heat = 1
  [../]
[]

[Kernels]
  [./hc]
    type = HeatConduction
    variable = temp
    use_displaced_mesh = true
    block = '1 2'
  [../]
[]

[Constraints]
  [./ced]
    type = GapConductanceConstraint
    variable = lm
    interface = middle
    master_variable = temp
    k = 100
    use_displaced_mesh = true
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = temp
    boundary = 'left'
    value = 1
  [../]

  [./right]
    type = DirichletBC
    variable = temp
    boundary = 'right'
    value = 0
  [../]

  [./mortar]
    type = HeatConductionBC
    variable = temp
    boundary = '100 101'
    use_displaced_mesh = true
  [../]
[]

[Preconditioning]
  [./fmp]
    type = SMP
    full = true
    solve_type = 'NEWTON'
  [../]
[]

[Executioner]
  type = Steady
  nl_rel_tol = 1e-11
[]

[Outputs]
  exodus = true
  show = 'temp disp_x disp_y'
[]
