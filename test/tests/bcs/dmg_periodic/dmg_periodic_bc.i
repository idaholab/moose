[Mesh]
  [dmg]
    type = DistributedRectilinearMeshGenerator
    dim = 2
    nx = 40
    ny = 40
    nz = 0
    xmax = 40
    ymax = 40
    zmax = 0
  []
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./periodic_dist]
    order = FIRST
    family = LAGRANGE
  [../]
  [./pid]
    order = CONSTANT
    family = monomial
  []
[]

[AuxKernels]
  [./pidaux]
    type = ProcessorIDAux
    variable = pid
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./forcing]
    type = GaussContForcing
    variable = u
  [../]

  [./dot]
    type = TimeDerivative
    variable = u
  [../]
[]

[AuxKernels]
  [./periodic_dist]
    type = PeriodicDistanceAux
    variable = periodic_dist
    point = '4 6 0'
  [../]
[]

[BCs]
  [./Periodic]
    [./all]
      variable = u
      auto_direction = 'x y'
    [../]
  [../]
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 20
  solve_type = NEWTON
  nl_rel_tol = 1e-12
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
