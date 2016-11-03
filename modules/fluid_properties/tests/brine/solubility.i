# Test BrineFluidProperties calculations of halite solubility
#
# Experimental data from Bodnar et al, Synthetic fluid inclusions in natural
# quartz, III. Determination of phase equilibrium properties in the system
# H2O-NaCl to 1000C and 1500 bars, Geocehmica et Cosmochemica Acta, 49,
# 1861-1873 (1985).
# Note that the average of the range quoted has been used for each point.
#
#  --------------------------------------------------------------
#  Temperature (C)               |   386.5  |   545.5  |   630
#  Temperature (K)               |  659.65  |  818.65  |  903.15
#  --------------------------------------------------------------
#  Expected values
#  --------------------------------------------------------------
#  Halite solubility             |   0.442  |  0.6085  |  0.7185
#  --------------------------------------------------------------
#  Calculated values
#  --------------------------------------------------------------
#  Halite solubility             |   0.448  |  0.6165  |  0.7279
#  --------------------------------------------------------------
#
# These results are within expected accuracy

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 3
  xmax = 3
[]

[Variables]
  [./dummy]
  [../]
[]

[AuxVariables]
  [./pressure]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 1e6
  [../]
  [./temperature]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./solubility]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[AuxKernels]
  [./solubility]
    type = MaterialRealAux
     variable = solubility
     property = solubility
  [../]
[]

[Functions]
  [./tic]
    type = ParsedFunction
    value = 'if(x<1, 659.65, if(x<2, 818.65, 903.15))'
  [../]
[]

[ICs]
  [./t_ic]
    type = FunctionIC
    function = tic
    variable = temperature
  [../]
[]

[Modules]
  [./FluidProperties]
    [./brine]
      type = BrineFluidProperties
    [../]
  [../]
[]

[Materials]
  [./fp_mat]
    type = BrineFluidPropertiesTestMaterial
    pressure = pressure
    temperature = temperature
    xnacl = 0
    fp = brine
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = dummy
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Postprocessors]
  [./solubility0]
    type = ElementalVariableValue
    variable = solubility
    elementid = 0
  [../]
  [./solubility1]
    type = ElementalVariableValue
    variable = solubility
    elementid = 1
  [../]
  [./solubility2]
    type = ElementalVariableValue
    variable = solubility
    elementid = 2
  [../]
[]

[Outputs]
  csv = true
  execute_on = 'timestep_end'
[]
