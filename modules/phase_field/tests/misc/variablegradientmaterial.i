[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 5
  ny = 5
  nz = 2
[]

[AuxVariables]
  [./c]
    [./InitialCondition]
      type = SmoothCircleIC
      x1 = 0
      y1 = 0
      z1 = 0
      invalue = 1
      outvalue = 0
      radius = 0.7
      int_width = 0.5
    [../]
  [../]
[]

[Materials]
  [./var_grad]
    type = VariableGradientMaterial
    prop = grad_c
    variable = c
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
  execute_on = final
[]
