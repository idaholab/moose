[GlobalParams]
  displacements = 'disp_r disp_z'
  large_kinematics = true
  stabilize_strain = true
[]

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
[]

[Problem]
  coord_type = RZ
[]

[Variables]
  [disp_r]
  []
  [disp_z]
  []
[]

[Kernels]
  [sdr]
    type = TotalLagrangianStressDivergenceAxisymmetricCylindrical
    variable = disp_r
    component = 0
  []
  [sdz]
    type = TotalLagrangianStressDivergenceAxisymmetricCylindrical
    variable = disp_z
    component = 1
  []
[]

[BCs]
  [bottom]
    type = DirichletBC
    preset = false
    variable = disp_z
    boundary = bottom
    value = 0.0
  []
  [top]
    type = FunctionNeumannBC
    variable = disp_z
    boundary = top
    function = 't*1e3'
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
    type = ComputeLagrangianStrainAxisymmetricCylindrical
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
