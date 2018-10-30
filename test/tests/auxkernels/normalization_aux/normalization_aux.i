[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
    initial_condition = 1.0
  [../]
[]

[AuxVariables]
  [./u_normalized]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff_u]
    type = Diffusion
    variable = u
  [../]
[]

[AuxKernels]
  [./normalization_auxkernel]
    type = NormalizationAux
    variable = u_normalized
    source_variable = u
    normal_factor = 2.0
    execute_on = timestep_end
    # Note: 'normalization' or 'shift' are provided as CLI args
  [../]
[]

[BCs]
  [./left_u]
    type = DirichletBC
    variable = u
    boundary = left
    value = 1
  [../]

  [./right_u]
    type = DirichletBC
    variable = u
    boundary = right
    value = 2
  [../]
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [./unorm]
    type = ElementIntegralVariablePostprocessor
    variable = u
    execute_on = 'initial timestep_end'
  [../]
  [./u_normalized_norm]
    type = ElementIntegralVariablePostprocessor
    variable = u_normalized
    execute_on = 'initial timestep_end'
  [../]
  [./u0]
    type = PointValue
    variable = u
    point = '0 0 0'
    execute_on = 'initial timestep_end'
  [../]
[]

[Outputs]
  exodus = true
[]
