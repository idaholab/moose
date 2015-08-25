[Mesh]
  file = square2.e
  uniform_refine = 3
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
    x_center = 0.1
    y_center = 0.3
    x_spread = 0.1
    y_spread = 0.1
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
    point = '0.2 0.3 0.0'
  [../]
[]

# This test verifies that autodirection works with an arbitrary
# regular orthogonal mesh
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

[Outputs]
  execute_on = 'timestep_end'
  file_base = out_auto_non_generated
  exodus = true
[]

