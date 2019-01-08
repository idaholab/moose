#
# This problem is taken from the Abaqus verification manual:
#   "1.5.1 Membrane patch test"
# The stress solution is given as:
#   xx = yy = 1600
#   zz = 800
#   xy = 400
#   yz = zx = 0
#
# Since the strain is 1e-3 in both directions, the new density should be
#   new_density = original_density * V_0 / V
#   new_density = 0.283 / (1 + 1e-3 + 1e-3) = 0.282435

[GlobalParams]
  displacements = 'disp_x disp_y'
  temperature = temp
[]

[Mesh]
  file = elastic_patch_rz.e
[]

[Variables]
  [./temp]
    initial_condition = 117.56
  [../]
[]

[Modules/TensorMechanics/Master/All]
  strain = SMALL
  incremental = true
  planar_formulation = PLANE_STRAIN
  add_variables = true
  generate_output = 'stress_xx stress_yy stress_zz stress_xy stress_yz stress_zx'
[]

[Kernels]
  [./heat]
    type = TimeDerivative
    variable = temp
  [../]
[]

[BCs]
  [./ur]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 10
    function = '1e-3*(x+0.5*y)'
  [../]
  [./uz]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 10
    function = '1e-3*(y+0.5*x)'
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.25
  [../]
  [./stress]
    type = ComputeStrainIncrementBasedStress
  [../]

  [./density]
    type = Density
    density = 0.283
    outputs = all
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  end_time = 1.0
[]

[Outputs]
  exodus = true
[]
