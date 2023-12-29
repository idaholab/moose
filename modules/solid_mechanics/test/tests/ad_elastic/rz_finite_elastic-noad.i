[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 3
[]

[Problem]
  coord_type = RZ
[]

[GlobalParams]
  displacements = 'disp_r disp_z'
[]

[Variables]
  # scale with one over Young's modulus
  [./disp_r]
    scaling = 1e-10
  [../]
  [./disp_z]
    scaling = 1e-10
  [../]
[]

[Kernels]
  [./stress_r]
    type = StressDivergenceRZTensors
    component = 0
    variable = disp_r
    use_displaced_mesh = true
  [../]
  [./stress_z]
    type = StressDivergenceRZTensors
    component = 1
    variable = disp_z
    use_displaced_mesh = true
  [../]
[]

[BCs]
  [./bottom]
    type = DirichletBC
    variable = disp_z
    boundary = bottom
    value = 0
  [../]
  [./axial]
    type = DirichletBC
    variable = disp_r
    boundary = left
    value = 0
  [../]
  [./rdisp]
    type = DirichletBC
    variable = disp_r
    boundary = right
    value = 0.1
  [../]
[]

[Materials]
  [./elasticity]
    type = ComputeIsotropicElasticityTensor
    poissons_ratio = 0.3
    youngs_modulus = 1e10
  [../]
  [./strain]
    type = ComputeAxisymmetricRZFiniteStrain
  [../]
  [./stress]
    type = ComputeFiniteStrainElasticStress
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
  dt = 0.05
  solve_type = 'NEWTON'

  petsc_options_iname = -pc_hypre_type
  petsc_options_value = boomeramg

  dtmin = 0.05
  num_steps = 1
[]

[Outputs]
  exodus = true
  file_base = rz_finite_elastic_out
[]
