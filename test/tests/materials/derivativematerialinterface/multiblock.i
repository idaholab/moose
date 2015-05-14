[Mesh]
  type = FileMesh
  file = rectangle.e
[]

[Variables]
  [./c]
  [../]
[]

[Materials]
  [./mat1]
    type = DefaultMatPropConsumerMaterial
    block = 1
  [../]
  [./mat2]
    type = DefaultMatPropConsumerMaterial
    block = 2
  [../]
  [./mat1b]
    type = DefaultMatPropConsumerMaterial
    mat_prop = prop2
    block = 1
  [../]
  [./mat2b]
    type = DefaultMatPropConsumerMaterial
    mat_prop = prop2
    block = 2
  [../]
[]

[Kernels]
  [./kern1]
    type = DefaultMatPropConsumerKernel
    variable = c
    block = 1
  [../]
  [./kern2]
    type = DefaultMatPropConsumerKernel
    variable = c
    block = 2
  [../]
  [./kern1b]
    type = DefaultMatPropConsumerKernel
    variable = c
    mat_prop = prop3
    block = 1
  [../]
  [./kern2b]
    type = DefaultMatPropConsumerKernel
    variable = c
    mat_prop = prop3
    block = 2
  [../]
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Outputs]
  output_initial = true
  exodus = true
  [./console]
    type = Console
    perf_log = false
    output_on = 'initial timestep_end failed nonlinear'
  [../]
[]
