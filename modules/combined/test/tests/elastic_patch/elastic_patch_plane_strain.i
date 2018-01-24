#
# This problem is taken from the Abaqus verification manual:
#   "1.5.1 Membrane patch test"
# The stress solution is given as:
#   xx = yy = 1600
#   zz = 800
#   xy = 400
#   yz = zx = 0
#

[GlobalParams]
  displacements = 'disp_x disp_y'
  temperature = temp
[]

[Mesh]
  file = elastic_patch_rz.e
[]

[Functions]
  [./ux]
    type = ParsedFunction
    value = '1e-3*(x+0.5*y)'
  [../]
  [./uy]
    type = ParsedFunction
    value = '1e-3*(y+0.5*x)'
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
  planar_formulation = PLANE_STRAIN
  add_variables = true
  generate_output = 'stress_xx stress_yy stress_zz stress_xy stress_yz stress_zx'
[]

[Kernels]
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
    function = ux
  [../]
  [./uz]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 10
    function = uy
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
  exodus = true
[]
