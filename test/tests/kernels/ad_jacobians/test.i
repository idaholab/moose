[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./damage_dt]
    type = ADTimeDerivative
    variable = u
  [../]
  [./damage]
    type = ADBodyForce
    value = 1
    variable = u
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
[]
