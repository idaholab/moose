[Mesh]
  file = 2blk-gap.e

  [./MortarInterfaces]
    [./middle]
      master = 100
      slave = 101
      subdomain = 1000
    [../]
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
    use_displaced_mesh = false
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
[]

[Preconditioning]
  [./fmp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  nl_rel_tol = 1e-11
[]

[Outputs]
  exodus = true
[]
