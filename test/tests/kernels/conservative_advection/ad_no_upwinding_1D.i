# ADConservativeAdvection with upwinding_type = None
# Apply a velocity = (1, 0, 0) and see a pulse advect to the right
# Note there are overshoots and undershoots
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
[]

[Variables]
  [u]
  []
[]

[BCs]
  [u_injection_left]
    type = InflowBC
    boundary = left
    variable = u
    velocity = '1 0 0'
    inlet_conc = 1
  []
[]

[Materials]
  [v]
    type = ADGenericConstantVectorMaterial
    prop_names = v
    prop_values = '1 0 0'
  []
[]

[Kernels]
  [udot]
    type = ADTimeDerivative
    variable = u
  []
  [advection]
    type = ADConservativeAdvection
    variable = u
  []
[]

[Executioner]
  type = Transient
  solve_type = LINEAR
  dt = 0.1
  end_time = 1
  l_tol = 1E-14
[]

[Outputs]
  exodus = true
[]
