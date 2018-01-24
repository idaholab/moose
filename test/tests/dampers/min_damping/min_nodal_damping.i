[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
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

[BCs]
  [./u_left]
    type = DirichletBC
    boundary = left
    variable = u
    value = 0.0
  [../]
[]

[Dampers]
  [./limit]
    type = BoundingValueNodalDamper
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
