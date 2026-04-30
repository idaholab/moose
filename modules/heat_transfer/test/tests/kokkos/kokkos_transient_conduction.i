[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [temp]
    initial_condition = 100.0
  []
[]

[Kernels]
  [heat_conduction_time]
    type = KokkosHeatConductionTimeDerivative
    variable = temp
    density = 1
  []
  [heat_conduction]
    type = KokkosHeatConduction
    variable = temp
  []
[]

[BCs]
  [left]
    type = KokkosDirichletBC
    variable = temp
    boundary = left
    value = 100.0
  []
  [right]
    type = KokkosDirichletBC
    variable = temp
    boundary = right
    value = 200.0
  []
[]

[Materials]
  [conduction]
    type = KokkosHeatConductionMaterial
    thermal_conductivity = 10
    specific_heat = 1000
  []
[]

[Executioner]
  type = Transient
  nl_rel_tol = 1e-8
  start_time = 0
  end_time = 100
  dt = 10
[]

[Outputs]
  exodus = true
[]
