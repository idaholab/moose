[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 100
  ny = 100
[]

[Problem]
  solve = false
[]

[AuxVariables]
  [u_vww]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[ICs]
  [u_vww]
    type = VolumeWeightedWeibull
    variable = u_vww
    reference_volume = 0.0001 #This is the volume of an element for a 100x100 mesh
    weibull_modulus = 15.0
    median = 1.0
# When the reference_volume is equal to the element volume (so that there is no volume correction),
# this combination of Weibull modulus and median gives the same distribution that you would get with
# the following parameters in a WeibullDistribution:
#    weibull_modulus = 15.0
#    location = 0
#    scale = 1.024735156 #median * (-1/log(0.5))^(1/weibull_modulus)
  []
[]

[VectorPostprocessors]
  [./histo]
    type = VariableValueVolumeHistogram
    variable = u_vww
    min_value = 0
    max_value = 2
    bin_number = 100
    execute_on = initial
    outputs = initial
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  [./initial]
    type = CSV
    execute_on = initial
  [../]
[]
