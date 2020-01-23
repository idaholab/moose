# Test of EquilibriumConstantAux with a single log(K) value.
# The resulting equilibrium constant should simple be constant.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
[]

[AuxVariables]
  [./logk]
  [../]
[]

[AuxKernels]
  [./logk]
    type = EquilibriumConstantAux
    temperature = temperature
    temperature_points = 300
    logk_points = 1.23
    variable = logk
  [../]
[]

[Variables]
  [./temperature]
  [../]
[]

[Kernels]
  [./temperature]
    type = Diffusion
    variable = temperature
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = temperature
    value = 150
    boundary = left
  [../]
  [./right]
    type = DirichletBC
    variable = temperature
    value = 400
    boundary = right
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
