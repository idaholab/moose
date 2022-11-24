[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[SolidProperties]
  [sp]
    type = ThermalSS316Properties
  []
[]

[Materials]
  [thermal_mat]
    type = ADThermalSolidPropertiesMaterial
    temperature = T
    sp = sp
    density = rho
    specific_heat = cp
    thermal_conductivity = k
  []
[]

[Variables]
  [T]
    order = FIRST
    family = LAGRANGE
  []
[]

[ICs]
  [T_ic]
    type = ConstantIC
    variable = T
    value = 300
  []
[]

[Kernels]
  [time_derivative]
    type = ADHeatConductionTimeDerivative
    variable = T
    density_name = rho
    specific_heat = cp
  []
  [heat_conduction]
    type = ADHeatConduction
    variable = T
    thermal_conductivity = k
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = T
    boundary = left
    value = 500
  []
[]

[Executioner]
  type = Transient

  scheme = implicit-euler
  dt = 100.0
  num_steps = 5
  abort_on_solve_fail = true

  solve_type = NEWTON
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10
  nl_max_its = 10

  l_tol = 1e-5
  l_max_its = 10
[]

[Outputs]
  exodus = true
[]
