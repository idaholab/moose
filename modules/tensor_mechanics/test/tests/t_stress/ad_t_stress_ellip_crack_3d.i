[GlobalParams]
  order = FIRST
  family = LAGRANGE
  displacements = 'disp_x disp_y disp_z'
  volumetric_locking_correction = true
[]

[Mesh]
  file = ellip_crack_4sym_norad_mm.e
  displacements = 'disp_x disp_y disp_z'
  partitioner = centroid
  centroid_partitioner_direction = z
[]

[AuxVariables]
  [SED]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Functions]
  [rampConstantUp]
    type = PiecewiseLinear
    x = '0. 1.'
    y = '0. 1.'
    scale_factor = -689.5 #MPa
  []
[]

[DomainIntegral]
  integrals = 'JIntegral InteractionIntegralKI InteractionIntegralT'
  boundary = 1001
  crack_direction_method = CurvedCrackFront
  crack_end_direction_method = CrackDirectionVector
  crack_direction_vector_end_1 = '0.0 1.0 0.0'
  crack_direction_vector_end_2 = '1.0 0.0 0.0'
  radius_inner = '12.5 25.0 37.5'
  radius_outer = '25.0 37.5 50.0'
  intersecting_boundary = '1 2'
  symmetry_plane = 2
  youngs_modulus = 206.8e+3 #MPa
  poissons_ratio = 0.3
  block = 1
  incremental = true
  use_automatic_differentiation = true
[]

[Modules/TensorMechanics/Master]
  [master]
    strain = FINITE
    add_variables = true
    incremental = true
    generate_output = 'stress_xx stress_yy stress_zz vonmises_stress strain_xx strain_yy strain_zz'
    use_automatic_differentiation = true
  []
[]

[AuxKernels]
  [SED]
    type = MaterialRealAux
    variable = SED
    property = strain_energy_density
    execute_on = timestep_end
  []
[]

[BCs]
  [crack_y]
    type = ADDirichletBC
    variable = disp_z
    boundary = 6
    value = 0.0
  []
  [no_y]
    type = ADDirichletBC
    variable = disp_y
    boundary = 12
    value = 0.0
  []
  [no_x]
    type = ADDirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  []
  [Pressure]
    [Side1]
      boundary = 5
      function = rampConstantUp
      use_automatic_differentiation = true
    []
  []
[]

[Materials]
  [elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 206.8e+3
    poissons_ratio = 0.3
  []
  [elastic_stress]
    type = ADComputeFiniteStrainElasticStress
  []
[]

[Executioner]
  type = Transient
  #petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      4'

  line_search = 'none'

  l_max_its = 50
  nl_max_its = 20
  nl_abs_tol = 1e-5
  nl_rel_tol = 1e-11
  l_tol = 1e-2

  start_time = 0.0
  dt = 1

  end_time = 1
  num_steps = 1
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = ad_t_stress_ellip_crack_out
  csv = true
[]
