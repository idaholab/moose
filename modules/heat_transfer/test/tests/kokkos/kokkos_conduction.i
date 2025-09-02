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

[KokkosKernels]
  [heat_conduction]
    type = KokkosHeatConduction
    variable = temp
  []
[]

[KokkosBCs]
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

[KokkosMaterials]
  [conduction]
    type = KokkosHeatConductionMaterial
    thermal_conductivity = 10
  []
[]

[Executioner]
  type = Steady
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
