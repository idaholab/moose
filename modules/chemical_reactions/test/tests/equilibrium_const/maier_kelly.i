# Test of EquilibriumConstantAux with eight log10(Keq) values.
# The resulting equilibrium constant should be a Maier-Kelly best fit.

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
    temperature_points = '273.16 298.15 333.15 373.15 423.15 473.15 523.15 573.15'
    logk_points = '-6.5804 -6.3447 -6.2684 -6.3882 -6.7235 -7.1969 -7.7868 -8.5280'
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
    value = 300
    boundary = left
  [../]
  [./right]
    type = DirichletBC
    variable = temperature
    value = 573.15
    boundary = right
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
