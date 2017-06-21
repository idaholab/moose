[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
[]

[AuxVariables]
  [./v]
  [../]
[]

[AuxKernels]
  [./v]
    type = NormalizationAux
    variable = v
    source_variable = u
    normalization = uint_scaled
    execute_on = timestep_end
  [../]
[]

[Postprocessors]
  [./uint]
    type = ElementIntegralVariablePostprocessor
    variable = u
    execute_on = timestep_end
  [../]
  [./uint_scaled]
    type = ScalePostprocessor
    value = uint
    scaling_factor = 2
    execute_on = timestep_end
  [../]
  [./uint_scaled_another]
    type = ScalePostprocessor
    value = uint
    scaling_factor = 2
    execute_on = timestep_end
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
