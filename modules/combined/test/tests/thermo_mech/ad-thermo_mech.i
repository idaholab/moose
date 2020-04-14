[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  temperature = temp
  volumetric_locking_correction = true
[]

[Mesh]
  file = cube.e
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]

  [./temp]
  [../]
[]

[Kernels]
  [./TensorMechanics]
    use_automatic_differentiation = true
  [../]

  [./heat]
    type = ADHeatConduction
    variable = temp
  [../]
[]

[BCs]
  [./bottom_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]
  [./bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = 1
    value = 0.0
  [../]
  [./bottom_z]
    type = DirichletBC
    variable = disp_z
    boundary = 1
    value = 0.0
  [../]

  [./bottom_temp]
    type = DirichletBC
    variable = temp
    preset = false
    boundary = 1
    value = 10.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 1.0
    poissons_ratio = 0.3
  [../]
  [./strain]
    type = ADComputeSmallStrain
    eigenstrain_names = eigenstrain
  [../]
  [./thermal_strain]
    type = ADComputeThermalExpansionEigenstrain
    stress_free_temperature = 0.0
    thermal_expansion_coeff = 1e-5
    eigenstrain_name = eigenstrain
  [../]
  [./stress]
    type = ADComputeLinearElasticStress
  [../]

  [./heat]
    type = ADHeatConductionMaterial
    specific_heat = 1.0
    thermal_conductivity = 1.0
  [../]

  [./density]
    type = ADDensity
    density = 1.0
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  nl_rel_tol = 1e-14
  l_tol = 1e-3

  l_max_its = 100

  dt = 1.0
  end_time = 1.0
[]

[Outputs]
  exodus = true
[]
