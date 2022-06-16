[GlobalParams]
  order = FIRST
  family = LAGRANGE
  displacements = 'disp_x disp_y disp_z'
  volumetric_locking_correction = true
[]

[Mesh]
  file = c_integral_coarse.e
  partitioner = centroid
  centroid_partitioner_direction = z
[]

[AuxVariables]
  [./SED]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./resid_z]
  [../]
[]

[Functions]
  [./rampConstantUp]
    type = PiecewiseLinear
    x = '0. 0.1 100.0'
    y = '0. 1 1'
    scale_factor = -68.95 #MPa
  [../]
  [./dts]
  type = PiecewiseLinear
  x = '0   1'
  y = '1   400000'
[../]
[]

[Modules/TensorMechanics/Master]
  [./master]
    strain = FINITE
    add_variables = true
    incremental = true
    generate_output = 'stress_xx stress_yy stress_zz vonmises_stress'
  [../]
[]

[AuxKernels]
  [./SED]
    type = MaterialRealAux
    variable = SED
    property = strain_energy_density
    execute_on = timestep_end
  [../]
[]

[BCs]
  [./crack_y]
    type = DirichletBC
    variable = disp_z
    boundary = 6
    value = 0.0
  [../]
  [./no_y]
    type = DirichletBC
    variable = disp_y
    boundary = 12
    value = 0.0
  [../]
  [./no_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]
  [./Pressure]
    [./Side1]
      boundary = 5
      function = rampConstantUp
    [../]
  [../]
[] # BCs

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 206800
    poissons_ratio = 0.0
  [../]
  [./radial_return_stress]
    type = ComputeMultipleInelasticStress
    inelastic_models = 'powerlawcrp'
  [../]
  [./powerlawcrp]
    type = PowerLawCreepStressUpdate
    coefficient = 3.125e-21 # 7.04e-17 #
    n_exponent = 4.0
    m_exponent = 0.0
    activation_energy = 0.0
    # max_inelastic_increment = 0.01
  [../]
[]

[DomainIntegral]
  integrals = CIntegral
  boundary = 1001
  crack_direction_method = CurvedCrackFront
  crack_end_direction_method = CrackDirectionVector
  crack_direction_vector_end_1 = '0.0 1.0 0.0'
  crack_direction_vector_end_2 = '1.0 0.0 0.0'
  radius_inner = '12.5 25.0 37.5'
  radius_outer = '25.0 37.5 50.0'
  intersecting_boundary = '1 2'
  symmetry_plane = 2
  incremental = true
  inelastic_models = 'powerlawcrp'
[]

[Executioner]

   type = Transient
  # Two sets of linesearch options are for petsc 3.1 and 3.3 respectively
  #Preconditioned JFNK (default)
  solve_type = 'NEWTON'

#  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      4'
  line_search = 'none'

   l_max_its = 50
   nl_max_its = 20
   nl_abs_tol = 1e-3
   nl_rel_tol = 1e-11
   l_tol = 1e-2

   start_time = 0.0
   end_time = 401

   [./TimeStepper]
     type = FunctionDT
     function = dts
     min_dt = 1.0
   [../]
[]

[Postprocessors]
  [./_dt]
    type = TimestepSize
  [../]
  [./nl_its]
    type = NumNonlinearIterations
  [../]
  [./lin_its]
    type = NumLinearIterations
  [../]
  [./react_z]
    type = NodalSum
    variable = resid_z
    boundary = 5
  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  csv = true
[]
