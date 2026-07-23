[GlobalParams]
  displacements = disp_x
[]

[Mesh]
  file = line.e
  coord_type = RZ
[]

[Variables]
  [disp_x]
  []
  [temp]
    initial_condition = 0
  []
[]

[Kernels]
  [heat]
    type = ADDiffusion
    variable = temp
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [gps]
    planar_formulation = GENERALIZED_PLANE_STRAIN
    scalar_out_of_plane_strain = scalar_strain_yy
    strain = SMALL
    use_automatic_differentiation = true
    eigenstrain_names = eigenstrain
    temperature = temp
  []
[]

[BCs]
  [no_x]
    type = DirichletBC
    boundary = 1
    variable = disp_x
    value = 0
  []
  [temp_left]
    type = DirichletBC
    boundary = 1
    variable = temp
    value = 0
  []
  [temp_right]
    type = DirichletBC
    boundary = 2
    variable = temp
    value = 1
  []
[]

[Materials]
  [elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 3600
    poissons_ratio = 0.2
  []
  [thermal_strain]
    type = ADComputeThermalExpansionEigenstrain
    thermal_expansion_coeff = 1e-3
    temperature = temp
    stress_free_temperature = 0
    eigenstrain_name = eigenstrain
  []
  [stress]
    type = ADComputeLinearElasticStress
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  line_search = none
  nl_abs_tol = 1e-12
  nl_rel_tol = 1e-10
  l_tol = 1e-12
  start_time = 0
  end_time = 1
  num_steps = 1
[]
