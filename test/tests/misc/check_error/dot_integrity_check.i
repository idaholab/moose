# Test that coupling a time derivative of a variable (DotCouplingKernel) and using a Steady executioner
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
    type = CoefDiffusion
    variable = u
    coef = 0.5
  [../]

  [./conv_v]
    type = DotCouplingKernel
    variable = v
    v = u
  [../]
[]

[Executioner]
  type = Steady
[]
