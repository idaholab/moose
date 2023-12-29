[GlobalParams]
  displacements = 'disp_r disp_z'
  large_kinematics = true
  stabilize_strain = true
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
[]

[Problem]
  coord_type = RZ
[]

[Variables]
  [disp_r]
    [InitialCondition]
      type = RandomIC
      min = 0
      max = 0.02
    []
  []
  [disp_z]
    [InitialCondition]
      type = RandomIC
      min = -0.02
      max = 0.02
    []
  []
  [temperature]
  []
[]

[Kernels]
  [sdr]
    type = TotalLagrangianStressDivergenceAxisymmetricCylindrical
    variable = disp_r
    component = 0
    temperature = temperature
    eigenstrain_names = "thermal_contribution"
  []
  [sdz]
    type = TotalLagrangianStressDivergenceAxisymmetricCylindrical
    variable = disp_z
    component = 1
    temperature = temperature
    eigenstrain_names = "thermal_contribution"
  []
  [temperature]
    type = Diffusion
    variable = temperature
  []
[]

[BCs]
  [bottom]
    type = DirichletBC
    variable = disp_z
    boundary = bottom
    value = 0.0
    preset = false
  []
  [top]
    type = DirichletBC
    variable = disp_z
    boundary = top
    value = 0.1
    preset = false
  []
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
    type = ComputeLagrangianStrainAxisymmetricCylindrical
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
