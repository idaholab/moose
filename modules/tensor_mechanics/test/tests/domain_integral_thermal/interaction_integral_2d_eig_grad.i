#This problem from [Wilson 1979] tests the thermal strain term in the
#interaction integral. In this variant of this test, rather than using the
#standard mechanism for applying thermal strain, the eigenstrain for the
#thermal strain is applied using a generic object, which also supplies its
#gradient. This gradient is used in the interaction integral, with a nearly
#identical result to that from the version of this test that applies that
#in the standard manner.
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
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = False
[]

[Mesh]
  file = crack2d.e
  displacements = 'disp_x disp_y'
#  uniform_refine = 3
[]

[Functions]
  [eigfunc]
    type = ParsedFunction
    expression = 1.35e-5*10.0*(2*x/504)
  []
[]

[DomainIntegral]
  integrals = 'InteractionIntegralKI'
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
  eigenstrain_gradient = thermal_expansion_gradient
[]

[Modules/TensorMechanics/Master]
  [all]
    strain = FINITE
    add_variables = true
    generate_output = 'stress_xx stress_yy stress_zz vonmises_stress'
    planar_formulation = PLANE_STRAIN
    eigenstrain_names = thermal_expansion
  []
[]

[BCs]
  [crack_y]
    type = DirichletBC
    variable = disp_y
    boundary = 100
    value = 0.0
  []
  [no_y]
    type = DirichletBC
    variable = disp_y
    boundary = 400
    value = 0.0
  []
  [no_x1]
    type = DirichletBC
    variable = disp_x
    boundary = 900
    value = 0.0
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 207000
    poissons_ratio = 0.3
  []
  [elastic_stress]
    type = ComputeFiniteStrainElasticStress
  []
  [thermal_expansion_strain]
    type = FunctionIsotropicEigenstrain
    function = eigfunc
    eigenstrain_name = thermal_expansion
  []
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
  [smp]
    type = SMP
    pc_side = left
    ksp_norm = preconditioned
    full = true
  []
[]
