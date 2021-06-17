[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  temperature = temp
[]

[Mesh]
  file = gap_heat_transfer_convex.e
[]

[Functions]
  [./disp]
    type = PiecewiseLinear
    x = '0 2.0'
    y = '0 1.0'
  [../]
  [./temp]
    type = PiecewiseLinear
    x = '0     1'
    y = '200 200'
  [../]
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]

  [./temp]
    initial_condition = 100
  [../]
[]

[ThermalContact]
  [./thermal_contact]
    type = GapHeatTransfer
    variable = temp
    primary = 2
    secondary = 3
    emissivity_primary = 0
    emissivity_secondary = 0
  [../]
[]

[Modules/TensorMechanics/Master/All]
  volumetric_locking_correction = true
  strain = FINITE
  eigenstrain_names = eigenstrain
[]

[Kernels]
  [./heat]
    type = HeatConduction
    variable = temp
  [../]
[]

[BCs]
  [./move_right]
    type = FunctionDirichletBC
    boundary = '3'
    variable = disp_x
    function = disp
  [../]

  [./fixed_x]
    type = DirichletBC
    boundary = '1'
    variable = disp_x
    value = 0
  [../]
  [./fixed_y]
    type = DirichletBC
    boundary = '1 2 3 4'
    variable = disp_y
    value = 0
  [../]
  [./fixed_z]
    type = DirichletBC
    boundary = '1 2 3 4'
    variable = disp_z
    value = 0
  [../]

  [./temp_bottom]
    type = FunctionDirichletBC
    boundary = 1
    variable = temp
    function = temp
  [../]
  [./temp_top]
    type = DirichletBC
    boundary = 4
    variable = temp
    value = 100
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    block = '1 2'
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  [../]
  [./thermal_strain]
    type = ComputeThermalExpansionEigenstrain
    stress_free_temperature = 100
    thermal_expansion_coeff = 0
    eigenstrain_name = eigenstrain
  [../]
  [./stress]
    type = ComputeFiniteStrainElasticStress
  [../]

  [./heat1]
    type = HeatConductionMaterial
    block = 1

    specific_heat = 1.0
    thermal_conductivity = 1.0
  [../]
  [./heat2]
    type = HeatConductionMaterial
    block = 2

    specific_heat = 1.0
    thermal_conductivity = 1.0
  [../]

  [./density]
    type = Density
    block = '1 2'
    density = 1.0
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  start_time = 0.0
  dt = 0.1
  end_time = 2.0
[]

[Outputs]
  exodus = true
[]
