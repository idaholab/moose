[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
[]

[Variables]
  [u]
    initial_condition = 1000.0
  []
[]

[Modules]
  [SolidProperties]
    [func]
      type = ThermalFunctionSolidProperties
      rho = '1000.0'
      cp = '200*t+150.0'
      k = '2.0*exp(-100.0/(2.0+t))+1.0'
    []
  []
[]

[Materials]
  [sp_mat]
    type = ThermalSolidPropertiesMaterial
    sp = func
    temperature = u
    output_properties = 'thermal_conductivity density specific_heat'
    outputs = exodus
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 1000.0
  []
  [right]
    type = DirichletBC
    variable = u
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
