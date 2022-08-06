[GlobalParams]
  displacements = 'disp_r disp_z'
  large_kinematics = false
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
    type = FunctionDirichletBC
    preset = false
    variable = disp_z
    boundary = top
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
    output_properties = 'pk1_stress'
    outputs = 'exodus'
  []
  [compute_strain]
    type = ComputeLagrangianStrainAxisymmetricCylindrical
    output_properties = 'mechanical_strain'
    outputs = 'exodus'
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
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
