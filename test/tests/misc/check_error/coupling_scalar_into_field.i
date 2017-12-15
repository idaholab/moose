[Mesh]
  type = GeneratedMesh
  dim = 2
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

  [./coupled]
    type = CoupledForce
    variable = u
    # this should trigger an error message, 'v' should a field variable
    v = a
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
