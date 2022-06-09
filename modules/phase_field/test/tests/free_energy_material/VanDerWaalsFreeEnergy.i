[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
  elem_type = QUAD4
[]

[Variables]
  [./c]
    [./InitialCondition]
      type = FunctionIC
      function = x*0.4+0.001
    [../]
  [../]
  [./T]
    [./InitialCondition]
      type = FunctionIC
      function = y*1999+1
    [../]
  [../]
[]

[Materials]
  [./free_energy]
    type =  VanDerWaalsFreeEnergy
    property_name = Fgas
    m = 134 # Xenon
    a = 7.3138
    b = 84.77
    omega = 41
    c = c
    T = T
    outputs = exodus
  [../]
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
