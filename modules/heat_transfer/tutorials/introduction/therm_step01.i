#
# Initial single block thermal input
# https://mooseframework.inl.gov/modules/heat_conduction/tutorials/introduction/therm_step01.html
#

[Mesh]
  [generated]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    xmax = 2
    ymax = 1
  []
[]

[Variables]
  [T]
  []
[]

[Kernels]
  [heat_conduction]
    type = HeatConduction
    variable = T
  []
[]

[Materials]
  [thermal]
    type = HeatConductionMaterial
    thermal_conductivity = 45.0
  []
[]

[Executioner]
  type = Transient
  end_time = 5
  dt = 1
[]

[Outputs]
  exodus = true
[]
