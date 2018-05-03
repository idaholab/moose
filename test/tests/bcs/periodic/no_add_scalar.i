# Test to make sure that periodic boundaries
# are not applied to scalar variables.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
[]

[Variables]
  [./c]
    [./InitialCondition]
      type = FunctionIC
      function = x
    [../]
  [../]
  [./scalar]
    family = SCALAR
  [../]
[]

[BCs]
  [./Periodic]
    [./all]
      auto_direction = x
    [../]
  [../]
[]

[Kernels]
  [./dt]
    type = TimeDerivative
    variable = c
  [../]
  [./diff]
    type = Diffusion
    variable = c
  [../]
[]

[ScalarKernels]
  [./scalar]
    type = ODETimeDerivative
    variable = scalar
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 3
[]

[Outputs]
  exodus = true
[]
