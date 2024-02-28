[GlobalParams]
  displacements = 'disp_r'
  large_kinematics = true
  stabilize_strain = true
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
[]

[Problem]
  coord_type = RSPHERICAL
[]

[Variables]
  [disp_r]
    [InitialCondition]
      type = RandomIC
      min = 0
      max = 0.02
    []
  []
  [temperature]
  []
[]

[Kernels]
  [sdr]
    type = TotalLagrangianStressDivergenceCentrosymmetricSpherical
    variable = disp_r
    component = 0
    temperature = temperature
    eigenstrain_names = "thermal_contribution"
  []
  [temperature]
    type = Diffusion
    variable = temperature
  []
[]

[BCs]
  [T_left]
    type = DirichletBC
    variable = temperature
    boundary = left
    value = 0
    preset = false
  []
  [T_right]
    type = DirichletBC
    variable = temperature
    boundary = right
    value = 1
    preset = false
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
    type = ComputeLagrangianStrainCentrosymmetricSpherical
    eigenstrain_names = 'thermal_contribution'
  []
  [thermal_expansion]
    type = ComputeThermalExpansionEigenstrain
    temperature = temperature
    thermal_expansion_coeff = 1.0e-3
    eigenstrain_name = thermal_contribution
    stress_free_temperature = 0.0
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  automatic_scaling = true
  end_time = 1
  dt = 1
[]
