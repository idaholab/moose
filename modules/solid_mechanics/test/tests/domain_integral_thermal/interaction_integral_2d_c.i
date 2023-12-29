#This problem from [Wilson 1979] tests the thermal strain term in the
#interaction integral
#
#theta_e = 10 degrees C; a = 252; E = 207000; nu = 0.3; alpha = 1.35e-5
#
#With uniform_refine = 3, KI converges to
#KI = 5.602461e+02 (interaction integral)
#KI = 5.655005e+02 (J-integral)
#
#Both are in good agreement with [Shih 1986]:
#average_value = 0.4857 = KI / (sigma_theta * sqrt(pi * a))
#sigma_theta = E * alpha * theta_e / (1 - nu)
# = 207000 * 1.35e-5 * 10 / (1 - 0.3) = 39.9214
#KI = average_value * sigma_theta * sqrt(pi * a) = 5.656e+02
#
#References:
#W.K. Wilson, I.-W. Yu, Int J Fract 15 (1979) 377-387
#C.F. Shih, B. Moran, T. Nakamura, Int J Fract 30 (1986) 79-102

[GlobalParams]
  order = FIRST
  family = LAGRANGE
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = False
[]

[Mesh]
  file = crack2d.e
  displacements = 'disp_x disp_y'
#  uniform_refine = 3
[]

[AuxVariables]
  [./SED]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./SERD]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./temp]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Functions]
  [./rampConstantUp]
    type = PiecewiseLinear
    x = '0. 0.1 100.0'
    y = '0. 1 1'
    scale_factor = -68.95 #MPa
  [../]
[]

[Modules/TensorMechanics/Master]
  [./master]
    strain = FINITE
    add_variables = true
    incremental = true
    generate_output = 'stress_xx stress_yy stress_zz vonmises_stress'
    planar_formulation = PLANE_STRAIN
  [../]
[]

[AuxKernels]
  [./SED]
    type = MaterialRealAux
    variable = SED
    property = strain_energy_density
    execute_on = timestep_end
  [../]
  [./SERD]
    type = MaterialRealAux
    variable = SERD
    property = strain_energy_rate_density
    execute_on = timestep_end
  [../]
[]

[BCs]
  [./crack_y]
    type = DirichletBC
    variable = disp_y
    boundary = 100
    value = 0.0
  [../]
  [./no_y]
    type = DirichletBC
    variable = disp_y
    boundary = 400
    value = 0.0
  [../]
  [./no_x1]
    type = DirichletBC
    variable = disp_x
    boundary = 900
    value = 0.0
  [../]
  [./Pressure]
    [./crack_pressure]
      boundary = 700
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
    n_exponent = 3.0
    m_exponent = 0.0
    activation_energy = 0.0
  [../]
[]

[DomainIntegral]
  integrals = 'CIntegral InteractionIntegralKI'
  boundary = 800
  crack_direction_method = CrackDirectionVector
  crack_direction_vector = '1 0 0'
  2d = true
  axis_2d = 2
  radius_inner = '60.0 80.0 100.0 120.0'
  radius_outer = '80.0 100.0 120.0 140.0'
  symmetry_plane = 1
  incremental = true

  # interaction integral parameters
  block = 1
  youngs_modulus = 207000
  poissons_ratio = 0.3
  inelastic_models = 'powerlawcrp'
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_ksp_type -sub_pc_type -pc_asm_overlap'
  petsc_options_value = 'asm         31   preonly   lu      1'

  line_search = 'none'

   l_max_its = 50
   nl_max_its = 40

   nl_rel_step_tol= 1e-10
   nl_rel_tol = 1e-10


   start_time = 0.0
   dt = 1

   end_time = 1
   num_steps = 1

[]

[Outputs]
  exodus = true
  csv = true
[]

[Preconditioning]
  active = 'smp'
  [./smp]
    type = SMP
    pc_side = left
    ksp_norm = preconditioned
    full = true
  [../]
[]
