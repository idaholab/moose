# Second time derivative of a coupled array variable with Newmark-beta time integration
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 3
[]

[Variables]
  [u]
    components = 2
  []
  [v]
    components = 2
  []
[]

[Kernels]
  [u_inertia]
    type = ADArrayCoupledTimeDerivatives
    variable = u
    v = v
    a = 0.5
    b = 1.0
  []
  [u_diff]
    type = ADArrayDiffusion
    variable = u
  []
  [v_inertia]
    type = ADArrayCoupledTimeDerivatives
    variable = v
    a = 0
    b = 1
  []
  [v_diff]
    type = ADArrayDiffusion
    variable = v
  []
[]

[BCs]
  [u]
    type = ADArrayDirichletBC
    variable = u
    boundary = 'left right'
    values = '0 1'
  []
  [v]
    type = ADArrayDirichletBC
    variable = v
    boundary = 'left right'
    values = '1 0'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  [TimeIntegrator]
    type = NewmarkBeta
  []
  dt = 0.1
  num_steps = 2
[]
