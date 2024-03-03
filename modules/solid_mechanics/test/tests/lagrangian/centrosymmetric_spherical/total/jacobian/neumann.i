[GlobalParams]
  displacements = 'disp_r'
  large_kinematics = true
  stabilize_strain = true
[]

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 5
  []
[]

[Problem]
  coord_type = RSPHERICAL
[]

[Variables]
  [disp_r]
  []
[]

[Kernels]
  [sdr]
    type = TotalLagrangianStressDivergenceCentrosymmetricSpherical
    variable = disp_r
    component = 0
  []
[]

[BCs]
  [top]
    type = FunctionDirichletBC
    preset = false
    variable = disp_r
    boundary = right
    function = 't'
  []
[]

[Materials]
  [elastic_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1000.0
    poissons_ratio = 0.25
  []
  [compute_stress]
    type = ComputeLagrangianLinearElasticStress
  []
  [compute_strain]
    type = ComputeLagrangianStrainCentrosymmetricSpherical
  []
[]

[Executioner]
  type = Transient

  solve_type = NEWTON

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  automatic_scaling = true

  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-10

  dt = 0.1
  num_steps = 5
[]
