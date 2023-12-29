# T-stress test for a through crack in a wide ("infinite") plate.
# For a finer mesh this problem converges to the solution T = -sigma.
# Ref: T.L. Anderson, Fracture Mechanics: Fundamentals and Applications

[GlobalParams]
  order = FIRST
  family = LAGRANGE
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = true
[]

[Mesh]
  file = crack_infinite_plate.e
  displacements = 'disp_x disp_y'
  partitioner = centroid
  centroid_partitioner_direction = z
[]

[AuxVariables]
  [./SED]
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

[DomainIntegral]
  integrals = 'JIntegral InteractionIntegralKI InteractionIntegralT'
  boundary = 1001
  crack_direction_method = CrackDirectionVector
  crack_direction_vector = '1 0 0'
  radius_inner = '0.06 0.08 0.10'
  radius_outer = '0.08 0.10 0.12'
  block = 1
  youngs_modulus = 30e+6
  poissons_ratio = 0.3
  2d = true
  axis_2d = 2
  symmetry_plane = 1
  incremental = true
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
[]

[BCs]
  [./no_x]
    type = DirichletBC
    variable = disp_x
    boundary = 300
    value = 0.0
  [../]
  [./no_y]
    type = DirichletBC
    variable = disp_y
    boundary = 100
    value = 0.0
  [../]
  [./Pressure]
    [./top]
      boundary = 200
      function = rampConstantUp
    [../]
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 30e+6
    poissons_ratio = 0.3
  [../]
  [./elastic_stress]
    type = ComputeFiniteStrainElasticStress
  [../]
[]

[Executioner]
   type = Transient

#  petsc_options = '-snes_ksp_ew'
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

[Outputs]
  file_base = t_stress_crack_infinite_plate_out
  csv = true
[]
