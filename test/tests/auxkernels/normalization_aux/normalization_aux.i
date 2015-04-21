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
    normalization = unorm
    normal_factor = 2.0
    execute_on = timestep_end
  [../]
[]

[BCs]
  [./left_u]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  [../]

  [./right_u]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 2
  [../]
[]

[Executioner]
  type = Steady

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
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
[]

[Outputs]
  exodus = true
  output_initial = true
[]
