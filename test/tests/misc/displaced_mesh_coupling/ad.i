[GlobalParams]
  displacements = 'u'
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [./u]
  [../]
  [./v]
  [../]
[]

[Kernels]
  [./u]
    type = ADDiffusion
    use_displaced_mesh = true
    variable = u
  [../]
  [./v]
    type = ADDiffusion
    use_displaced_mesh = false
    variable = v
  [../]
[]

[BCs]
  [./no_x]
    type = ADNeumannBC
    variable = u
    boundary = left
    value = 1.0e-3
    use_displaced_mesh = true
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
  [./lright]
    type = DirichletBC
    variable = v
    boundary = right
    value = 1
  [../]
[]

[Preconditioning]
  [./SMP]
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
