
[GlobalParams]
  displacements = 'disp_x disp_y'
  temperature = temp
  out_of_plane_strain = strain_zz
  full_jacobian = true
[]

[Mesh]
  type = PeridynamicsMesh
  horizon_number = 3

  [./gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
  [../]
  [./gpd]
    type = MeshGeneratorPD
    input = gmg
    retain_fe_mesh = false
  [../]
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]

  [./strain_zz]
  [../]

  [./temp]
  [../]
[]

[Modules/Peridynamics/Mechanics/Master]
  [./all]
    formulation = NONORDINARY_STATE
    stabilization = BOND_HORIZON_I
    eigenstrain_names = thermal_strain
  [../]
[]

[Kernels]
  [./strain_zz]
    type = WeakPlaneStressNOSPD
    variable = strain_zz
    eigenstrain_names = thermal_strain
  [../]

  [./heat_conduction]
    type = HeatConductionBPD
    variable = temp
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 2e5
    poissons_ratio = 0.3
  [../]
  [./strain]
    type = ComputePlaneSmallStrainNOSPD
    stabilization = BOND_HORIZON_I
    eigenstrain_names = thermal_strain
  [../]
  [./thermal_strain]
    type = ComputeThermalExpansionEigenstrain
    thermal_expansion_coeff = 1e-5
    stress_free_temperature = 0.5
    eigenstrain_name = thermal_strain
  [../]

  [./stress]
    type = ComputeLinearElasticStress
  [../]

  [./thermal]
    type = ThermalConstantHorizonMaterialBPD
    thermal_conductivity = 1.0
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_type'
    petsc_options_value = 'bcgs bjacobi test'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  end_time = 1
  dt = 1
  num_steps = 1

  [./Quadrature]
    type = GAUSS_LOBATTO
    order = FIRST
  [../]
[]
