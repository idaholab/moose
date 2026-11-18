[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
[]

[Variables]
  [u]
  []
  [v]
  []
[]

[Kernels]
  [diff_u]
    type = Diffusion
    variable = u
  []
  # couples u to v, so the problem is only solved in a single Newton iteration
  # if the off-diagonal Jacobian block is included
  [force_u]
    type = CoupledForce
    variable = u
    v = v
  []
  [diff_v]
    type = Diffusion
    variable = v
  []
[]

[BCs]
  [left_u]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [left_v]
    type = DirichletBC
    variable = v
    boundary = left
    value = 0
  []
  [right_v]
    type = DirichletBC
    variable = v
    boundary = right
    value = 1
  []
[]

[Preconditioning]
  # does not set 'full', which should then default to true for a NEWTON solve
  [smp]
    type = SMP
    petsc_options_iname = '-pc_type'
    petsc_options_value = 'lu'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Postprocessors]
  [nl_its]
    type = NumNonlinearIterations
  []
[]

[Outputs]
  csv = true
[]
