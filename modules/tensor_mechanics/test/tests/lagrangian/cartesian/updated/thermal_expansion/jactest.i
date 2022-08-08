[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 2
  ny = 2
  nz = 2
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  large_kinematics = false
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
  [temperature]
  []
[]

[Kernels]
  [sdx]
    type = UpdatedLagrangianStressDivergence
    variable = disp_x
    component = 0
    temperature = temperature
    eigenstrain_names = "thermal_contribution"
    use_displaced_mesh = false
  []
  [sdy]
    type = UpdatedLagrangianStressDivergence
    variable = disp_y
    component = 1
    temperature = temperature
    eigenstrain_names = "thermal_contribution"
    use_displaced_mesh = false
  []
  [sdz]
    type = UpdatedLagrangianStressDivergence
    variable = disp_z
    component = 2
    temperature = temperature
    eigenstrain_names = "thermal_contribution"
    use_displaced_mesh = false
  []
  [temperature]
    type = Diffusion
    variable = temperature
  []
[]

[Materials]
  [elastic_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 100000.0
    poissons_ratio = 0.3
  []
  [compute_stress]
    type = ComputeLagrangianLinearElasticStress
  []
  [compute_strain]
    type = ComputeLagrangianStrain
    eigenstrain_names = "thermal_contribution"
  []
  [thermal_expansion]
    type = ComputeThermalExpansionEigenstrain
    temperature = temperature
    thermal_expansion_coeff = 1.0e-3
    eigenstrain_name = thermal_contribution
    stress_free_temperature = 0.0
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
    petsc_options_iname = '-snes_type'
    petsc_options_value = 'test'
  []
[]

[Executioner]
  solve_type = NEWTON
  end_time = 1
  dt = 1
  type = Transient
[]

[Outputs]
  exodus = true
[]
