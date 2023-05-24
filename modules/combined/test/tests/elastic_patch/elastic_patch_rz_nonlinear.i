#
# This problem is taken from the Abaqus verification manual:
#   "1.5.4 Patch test for axisymmetric elements"
# The stress solution is given as:
#   xx = yy = zz = 19900
#   xy = 0
#
# If strain = log(1+1e-2) = 0.00995033...
# then
# stress = E/(1+PR)/(1-2*PR)*(1-PR +PR +PR)*strain = 19900.6617
# with E = 1e6 and PR = 0.25.
#
# The code computes stress = 19900.6617 when
# increment_calculation = eigen.  There is a small error when the
# rashidapprox option is used.
#
# Since the strain is 1e-3 in all three directions, the new density should be
#   new_density = original_density * V_0 / V
#   new_density = 0.283 / (1 + 9.95e-3 + 9.95e-3 + 9,95e-3) = 0.2747973
#
# The code computes a new density of .2746770


[GlobalParams]
  displacements = 'disp_x disp_y'
  temperature = temp
[]

[Problem]
  coord_type = RZ
[]

[Mesh]
  file = elastic_patch_rz.e
[]

[Variables]
  [temp]
    initial_condition = 117.56
  []
[]

[Modules/TensorMechanics/Master/All]
  strain = FINITE
  decomposition_method = EigenSolution
  add_variables = true
  generate_output = 'stress_xx stress_yy stress_zz stress_xy stress_yz stress_zx'
[]

[Kernels]
  [heat]
    type = TimeDerivative
    variable = temp
  []
[]

[BCs]
  [ur]
    type = FunctionDirichletBC
    variable = disp_x
    preset = false
    boundary = 10
    function = '1e-2*x'
  []
  [uz]
    type = FunctionDirichletBC
    variable = disp_y
    preset = false
    boundary = 10
    function = '1e-2*y'
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.25
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
  []

  [density]
    type = Density
    density = 0.283
    outputs = all
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton

  end_time = 1.0
[]

[Outputs]
  exodus = true
[]
