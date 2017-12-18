#
# This problem is taken from the Abaqus verification manual:
#   "1.5.4 Patch test for axisymmetric elements"
# The stress solution is given as:
#   xx = yy = zz = 2000
#   xy = 400
#

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

[Functions]
  [./ur]
    type = ParsedFunction
    value = '1e-3*x'
  [../]
  [./uz]
    type = ParsedFunction
    value = '1e-3*(x+y)'
  [../]
  [./body]
    type = ParsedFunction
    value = '-400/x'
  [../]
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]

  [./temp]
    initial_condition = 117.56
  [../]
[]

[Modules/TensorMechanics/Master/All]
  strain = SMALL
  incremental = true
  eigenstrain_names = eigenstrain
  add_variables = true
  generate_output = 'stress_xx stress_yy stress_zz stress_xy stress_yz stress_zx'
[]

[Kernels]
  [./body]
    type = BodyForce
    variable = disp_y
    value = 1
    function = body
  [../]

  [./heat]
    type = HeatConduction
    variable = temp
  [../]
[]

[BCs]
  [./ur]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 10
    function = ur
  [../]
  [./uz]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 10
    function = uz
  [../]

  [./temp]
    type = DirichletBC
    variable = temp
    boundary = 10
    value = 117.56
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.25
  [../]
  [./thermal_strain]
    type = ComputeThermalExpansionEigenstrain
    stress_free_temperature = 117.56
    thermal_expansion_coeff = 0.0
    eigenstrain_name = eigenstrain
  [../]
  [./stress]
    type = ComputeStrainIncrementBasedStress
  [../]

  [./heat]
    type = HeatConductionMaterial
    specific_heat = 0.116
    thermal_conductivity = 4.85e-4
  [../]

  [./density]
    type = Density
    density = 0.283
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  start_time = 0.0
  end_time = 1.0
[]

[Outputs]
  file_base = elastic_patch_rz_out
  [./out]
    type = Exodus
    elemental_as_nodal = true
  [../]
[]
