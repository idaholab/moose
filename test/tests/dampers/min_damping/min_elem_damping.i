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
    type = BoundingValueElementDamper
    variable = u
    max_value = 1.5
    min_value = -1.5
    min_damping = 0.001
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 2
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
