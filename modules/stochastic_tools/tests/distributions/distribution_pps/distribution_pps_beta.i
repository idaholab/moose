[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 2
  ny = 2
[]

[AuxVariables]
  [./v]
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[Functions]
  [./constant_func]
    type = ConstantFunction
    value = 2.798
  [../]
[]


[ICs]
  [./u_ic]
    type = ConstantIC
    variable = u
    value = 2
  [../]
[]

[AuxKernels]
  [./one]
    type = ConstantAux
    variable = v
    value = 1
    execute_on = 'initial timestep_end'
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
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Distributions]
  [./beta1]
    type = BetaDistribution
    seed = 20021986
    alpha = 0.5
    beta = 0.5
    lower_bound = 0.1
    upper_bound = 0.9
    scale = 1.0
  [../]
[]

[Postprocessors]
  [./value1]
    type = TestDistributionPostprocessor
    distribution = beta1
    sample_value = 0.5
    cdf_value = 0.2
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 20
[]

[Outputs]
  csv = true
[]

[Problem]
  solve = false
[]
