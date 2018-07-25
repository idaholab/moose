# ConservativeAdvection with upwinding_type = full
# Apply a velocity = (1, 0, 0) and see a pulse advect to the right
# Note that the pulse diffuses more than with no upwinding,
# but there are no overshoots and undershoots and that the
# center of the pulse at u=0.5 advects with the correct velocity
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
[]

[Variables]
  [./u]
  [../]
[]

[BCs]
  [./u_injection_left]
    type = InflowBC
    boundary = left
    variable = u
    velocity = '1 0 0'
    inlet_conc = 1
  [../]
[]

[Kernels]
  [./udot]
    type = MassLumpedTimeDerivative
    variable = u
  [../]
  [./advection]
    type = ConservativeAdvection
    variable = u
    velocity = '1 0 0'
    upwinding_type = full
  [../]
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
