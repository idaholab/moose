#
# Patch test for 1D spherical elements
#
# The 1D mesh is pinned at x=0.  The displacement at the outer node is set to
#   3e-3*X where X is the x-coordinate of that node.  That gives a strain of
#   3e-3 for the x, y, and z directions.
#
# Young's modulus is 1e6, and Poisson's ratio is 0.25.  This gives:
#
# Stress xx, yy, zz = E/(1+nu)/(1-2nu)*strain*((1-nu) + nu + nu) = 6000
#

[GlobalParams]
  displacements = 'disp_x'
  temperature = temp
[]

[Mesh]
  file = elastic_patch_rspherical.e
  coord_type = RSPHERICAL
[]

[Variables]
  [disp_x]
  []

  [temp]
    initial_condition = 117.56
  []
[]

[Physics/SolidMechanics/QuasiStatic/All]
  strain = SMALL
  incremental = true
  add_variables = true
  generate_output = 'stress_xx stress_yy stress_zz'
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
    boundary = '1 2'
    function = '3e-3*x'
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.25
  []
  [stress]
    type = ComputeStrainIncrementBasedStress
  []
[]

[Materials]
  [density]
    type = ADDensity
    density = 0.283
    outputs = all
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  end_time = 1.0
[]

[Outputs]
  exodus = true
[]
