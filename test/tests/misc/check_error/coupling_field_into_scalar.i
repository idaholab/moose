[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[AuxVariables]
  [./v]
  [../]
[]

[Variables]
  [./u]
  [../]
  [./a]
    family = SCALAR
    order = FIRST
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./slm]
    type = ScalarLagrangeMultiplier
    variable = u
    # this should trigger an error message, lambda is scalar
    lambda = v
  [../]
[]

[ScalarKernels]
  [./alpha]
    type = AlphaCED
    variable = a
    value = 1
  [../]
[]

[BCs]
  [./all]
    type = DirichletBC
    boundary = 'left right top bottom'
    variable = u
    value = 0
  [../]
[]

[Executioner]
  type = Steady
[]
