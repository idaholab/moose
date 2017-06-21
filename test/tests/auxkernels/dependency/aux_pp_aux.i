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
  [./t]
  [../]
[]

[AuxKernels]
  [./v]
    type = ParsedAux
    variable = v
    function = u+1
    args = u
    execute_on = timestep_end
  [../]
  [./t]
    type = NormalizationAux
    variable = t
    source_variable = v
    normalization = vint
    execute_on = timestep_end
  [../]
[]

[Postprocessors]
  [./vint]
    type = ElementIntegralVariablePostprocessor
    variable = v
    execute_on = timestep_end
  [../]
  [./vint_another]
    type = ElementIntegralVariablePostprocessor
    variable = v
    execute_on = timestep_end
  [../]
  [./tint]
    type = ElementIntegralVariablePostprocessor
    variable = t
    execute_on = timestep_end
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
