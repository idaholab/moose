# This test solves a 1D transient heat equation with a complicated thermal
# conductivity in order to verify jacobian calculation via AD

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 3
[]

[Variables]
  [./T]
    initial_condition = 1
  [../]
  [./c]
    initial_condition = 1
  [../]
[]

[ADKernels]
  [./HeatDiff]
    type = ADHeatConduction
    variable = T
    thermal_conductivity = thermal_conductivity
  [../]
  [./heat_dt]
    type = ADHeatConductionTimeDerivative
    variable = T
    specific_heat = thermal_conductivity
    density_name = thermal_conductivity
  [../]
  [./c]
    type = ADDiffusion
    variable = c
  [../]
[]

[ADMaterials]
  [./k]
    type = ADThermalConductivityTest
    c = c
    temperature = T
  [../]
[]

[Preconditioning]
  [./full]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
