# Test that coupling a time derivative of a variable and using a Steady executioner
# errors out

[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[Variables]
  [./v]
  [../]

  [./u]
  [../]
[]

[Kernels]
  [./diff_v]
    type = Diffusion
    variable = u
    coeff = 0.5
  [../]

  [./conv_v]
    type = CoupledTimeDerivative
    variable = v
    v = u
    coeff = -1
  [../]
[]

[Executioner]
  type = Steady
[]
