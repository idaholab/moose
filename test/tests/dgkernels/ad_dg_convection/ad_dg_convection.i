[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
  xmax = 20
[]

[Variables]
  [u]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Kernels]
  [time]
    type = TimeDerivative
    variable = u
  []
[]

[DGKernels]
  [convection]
    type = ADDGAdvection
    variable = u
    velocity = velocity
  []
[]

[BCs]
  [left]
    type = PenaltyDirichletBC
    value = 1
    penalty = 1e6
    boundary = 'left'
    variable = u
  []
[]

[Materials]
  [vel]
    type = ADGenericConstantVectorMaterial
    prop_names = 'velocity'
    prop_values = '1 0 0'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  num_steps = 10
  dt = 1
  dtmin = 1
[]
