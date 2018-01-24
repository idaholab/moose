[Mesh]
  file = 3blk.e
[]

[Variables]
  [./temp]
    order = FIRST
    family = LAGRANGE
    block = '1 2 3'
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

  [./middle]
    type = HeatConductionMaterial
    block = 3
    thermal_conductivity = 100
    specific_heat = 1
  [../]
[]

[Kernels]
  [./hc]
    type = HeatConduction
    variable = temp
    use_displaced_mesh = false
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

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  nl_rel_tol = 1e-11
  l_tol = 1e-11
[]

[Outputs]
  exodus = true
[]
