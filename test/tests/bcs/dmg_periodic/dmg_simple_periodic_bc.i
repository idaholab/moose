[Mesh]
  [dmg]
    type = DistributedRectilinearMeshGenerator
    dim = 3
    nx = 10
    ny = 10
    nz = 10
    xmax = 1
    ymax = 1
    zmax = 1
  []
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [forcing]
    type = BodyForce
    variable = u
  []
  [./dot]
    type = TimeDerivative
    variable = u
  [../]
[]

[BCs]
  [./Periodic]
    [./all]
      variable = u
      auto_direction = 'x y z'
    [../]
  [../]
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 5
  solve_type = NEWTON
  nl_rel_tol = 1e-10
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
