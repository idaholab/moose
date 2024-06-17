# Exception test: the incorrect number of initial stress functions are supplied

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 10
  xmin = -0.5
  xmax = 0.5
  ymin = -0.5
  ymax = 0.5
  zmin = -10
  zmax = 0
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]


[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]

[Kernels]
  [SolidMechanics]
    displacements = 'disp_x disp_y disp_z'
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1
    poissons_ratio = 0.25
  [../]
  [./strain]
    type = ComputeIncrementalStrain
    eigenstrain_names = ini_stress
  [../]
  [./ini_stress]
    type = ComputeEigenstrainFromInitialStress
    initial_stress = '1 0 1'
    eigenstrain_name = ini_stress
  [../]
  [./stress]
    type = ComputeFiniteStrainElasticStress
  [../]
[]


[Executioner]
  num_steps = 1
  solve_type = NEWTON
  type = Transient
[]
