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
    type = Diffusion
    use_displaced_mesh = true
    variable = u
  [../]
  [./v]
    type = Diffusion
    use_displaced_mesh = false
    variable = v
  [../]
[]

[BCs]
  [./no_x]
    type = NeumannBC
    variable = u
    boundary = left
    value = 1.0e-3
    use_displaced_mesh = true
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
[]
