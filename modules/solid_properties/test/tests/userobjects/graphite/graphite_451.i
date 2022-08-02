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
    [graphite]
      type = ThermalGraphiteProperties
      grade = H_451
    []
  []
[]

[Materials]
  [solid]
    type = ThermalSolidPropertiesMaterial
    sp = graphite
    temperature = T
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
