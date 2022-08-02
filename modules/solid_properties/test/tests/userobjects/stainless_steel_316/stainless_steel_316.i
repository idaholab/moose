[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 2
  []
[]

[Variables]
  [T]
    initial_condition = 1000.0
  []
[]

[Modules]
  [SolidProperties]
    [steel]
      type = ThermalSS316Properties
    []
  []
[]

[Materials]
  [sp_mat]
    type = ThermalSolidPropertiesMaterial
    temperature = T
    sp = steel
    output_properties = 'thermal_conductivity density specific_heat'
    outputs = exodus
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = T
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = T
    boundary = 'left'
    value = 1000.0
  []
  [right]
    type = DirichletBC
    variable = T
    boundary = 'right'
    value = 500.0
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
