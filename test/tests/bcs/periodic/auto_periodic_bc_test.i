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
  # This test will not work in parallel with ParallelMesh enabled
  # due to a bug in PeriodicBCs.
  distribution = serial
[]

[Variables]
  active = 'u'

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
    function = forcing_func
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
[]

[Output]
  file_base = out_auto
  interval = 1
  exodus = true
  perf_log = true
[]

