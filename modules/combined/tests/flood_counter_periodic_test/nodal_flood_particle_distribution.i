# This test places a single large feature in the middle of a square domain.
# The domain is sized such that individual element areas are uniform and equal
# to 1.0. The feature is a circle with a radius of 10.0.
#
# The area of the feature is calculated to be:
# pi * 10^2 ~ 3.14e+2
#
# The mesh is very coarse for this simulation so the calculated result
# based on a flood value of 0.5 is 3.16e+2

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 40
  ny = 40
  nz = 0

  xmax = 40
  ymax = 40
  zmax = 0
  elem_type = QUAD4
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = SmoothCircleIC
      x1 = 20
      y1 = 20
      radius = 10
      int_width = 1
      invalue = 1
      outvalue = 0
    [../]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./dot]
    type = TimeDerivative
    variable = u
  [../]
[]

[Postprocessors]
  active = 'bubbles'

  [./bubbles]
    type = FeatureFloodCount
    variable = u
    threshold = 0.5
    bubble_volume_file = nodal_flood_particle_distribution.csv
    execute_on = 'initial timestep_end'
    flood_entity_type = ELEMENTAL
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 2
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = nodal_flood_particle_distribution
  exodus = true
[]
