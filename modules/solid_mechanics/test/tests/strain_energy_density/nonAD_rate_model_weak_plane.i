# Single element test to check the strain energy density calculation

[GlobalParams]
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = true
  out_of_plane_strain = strain_zz
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 2
[]

[Variables]
  [./strain_zz]
  []
[]

[AuxVariables]
  [./SERD]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Functions]
  [./rampConstantUp]
    type = PiecewiseLinear
    x = '0. 1.'
    y = '0. 1.'
    scale_factor = -100
  [../]
[]

[Modules/TensorMechanics/Master]
  [./master]
    strain = FINITE
    add_variables = true
    incremental = true
    generate_output = 'stress_xx stress_yy stress_zz vonmises_stress strain_xx strain_yy'
    planar_formulation = WEAK_PLANE_STRESS
  [../]
[]

[AuxKernels]
  [./SERD]
    type = MaterialRealAux
    variable = SERD
    property = strain_energy_rate_density
    execute_on = timestep_end
  [../]
[]

[BCs]
  [./no_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'left'
    value = 0.0
  [../]
  [./no_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'bottom'
    value = 0.0
  [../]
  [./Pressure]
    [./top]
      boundary = 'top'
      function = rampConstantUp
    [../]
  [../]
[]

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
  [./strain_energy_rate_density]
    type = StrainEnergyRateDensity
    inelastic_models = 'powerlawcrp'
  [../]
[]

[Executioner]
   type = Transient

  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      4'

  line_search = 'none'

   l_max_its = 50
   nl_max_its = 20
   nl_abs_tol = 3e-7
   nl_rel_tol = 1e-12
   l_tol = 1e-2

   start_time = 0.0
   dt = 1

   end_time = 1
   num_steps = 1
[]

[Postprocessors]
  [./SERD]
    type = ElementAverageValue
    variable = SERD
  [../]
[]

[Outputs]
  csv = true
[]
