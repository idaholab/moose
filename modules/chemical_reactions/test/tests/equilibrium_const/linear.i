# Test of EquilibriumConstantAux with three log(K) values.
# The resulting equilibrium constant should be a linear best fit.

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
    temperature_points = '200 300 400'
    logk_points = '1.8 1.5 1.2'
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
