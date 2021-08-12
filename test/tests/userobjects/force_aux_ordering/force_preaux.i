[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  nx = 2
  ymin = 0
  ymax = 1
  ny = 2
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
    initial_condition = 1
  [../]
[]

[Kernels]
  [./time]
    type = TimeDerivative
    variable = u
  [../]

  [./diff]
    type = Diffusion
    variable = u
  [../]
[]


[Postprocessors]
  [./total_u]
    type = ElementIntegralVariablePostprocessor
    variable = u
  [../]

  # scale1 and scale2 depend on the ElementUO total_u. total_u is executed on
  # timestep_end in POST_AUX _before_ the GeneralPostprocessors. scale1 is executed
  # at its default location, timestep_end/POST_AUX/after total_u and hence gets
  # the most up to date information. scale2 is pushed into PRE_AUX and hence picks
  # up the value of total_u from the last timestep.
  [./scale1]
    type = ScalePostprocessor
    value = total_u
    scaling_factor = 1
  [../]

  [./scale2]
    type = ScalePostprocessor
    value = total_u
    scaling_factor = 1
    force_preaux = true
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]
[]

[Executioner]
  type = Transient
  dt = 1.0
  end_time = 2.0
[]

[Outputs]
  csv = true
[]
