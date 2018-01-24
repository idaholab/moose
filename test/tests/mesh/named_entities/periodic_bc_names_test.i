[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 50
  ny = 50
  nz = 0
  xmax = 40
  ymax = 40
  zmax = 0
  elem_type = QUAD4
  # This test will not work in parallel with DistributedMesh enabled
  # due to a bug in PeriodicBCs.
  parallel_type = replicated
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff forcing dot'

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

[BCs]
  [./Periodic]
    [./x]
      variable = u
      primary = 'left'
      secondary = 'right'
      translation = '40 0 0'
    [../]

    [./y]
      variable = u
      primary = 'bottom'
      secondary = 'top'
      translation = '0 40 0'
    [../]
  [../]
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 20
  solve_type = NEWTON
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
