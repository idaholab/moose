[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./u_dt]
    type = TimeDerivative
    variable = u
  [../]
  [./u_source]
    type = BodyForce
    variable = u
    value = 1
  [../]
[]

[Dampers]
  [./limit]
    type = ConstantDamper
    damping = 0.25
    min_damping = 0.5
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 2
  dt = 1.0
  dtmin = 0.5
[]

[Postprocessors]
  [./u_avg]
    type = ElementAverageValue
    variable = u
  [../]
  [./dt]
    type = TimestepSize
  [../]
[]
