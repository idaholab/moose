# This is a test of the ConvectiveHeatFluxBC.
# There is a single 1x1 element with a prescribed temperature
# on the left side and a convective flux BC on the right side.
# The temperature on the left is 100, and the far-field temp is 200.
# The conductance of the body (conductivity * length) is 10
#
# If the conductance in the BC is also 10, the temperature on the
# right side of the solid element should be 150 because half of the
# temperature drop should occur over the body and half in the BC.
#
# The integrated flux is deltaT * conductance, or -50 * 10 = -500.
# The negative sign indicates that heat is going into the body.

[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[Problem]
  extra_tag_vectors = 'bcs'
[]

[Variables]
  [./temp]
    initial_condition = 100.0
  [../]
[]

[Kernels]
  [./heat_conduction]
    type = ADHeatConduction
    variable = temp
    thermal_conductivity = 10
  [../]
[]

[BCs]
  [./left]
    type = ADDirichletBC
    variable = temp
    boundary = left
    value = 100.0
  [../]
  [./right]
    type = ADConvectiveHeatFluxBC
    variable = temp
    boundary = right
    T_infinity = 200.0
    heat_transfer_coefficient = 10
  [../]
[]

[Postprocessors]
  [./right_flux]
    type = SideDiffusiveFluxAverage
    variable = temp
    boundary = right
    diffusivity = 10
  [../]
[]

[Executioner]
  type = Transient

  num_steps = 1.0
  nl_rel_tol = 1e-12
[]

[Outputs]
  csv = true
[]
