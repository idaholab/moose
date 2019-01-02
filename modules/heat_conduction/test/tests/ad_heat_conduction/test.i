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
  [../]
  [./c]
  [../]
[]

[ICs]
  [./T_IC]
    type = RandomIC
    variable = T
  [../]
  [./c_IC]
    type = RandomIC
    variable = c
  [../]
[]

[ADKernels]
  [./HeatDiff]
    type = ADHeatConduction
    variable = T
    thermal_conductivity = thermal_conductivity
  [../]
[]

[Kernels]
  [./c]
    type = Diffusion
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
