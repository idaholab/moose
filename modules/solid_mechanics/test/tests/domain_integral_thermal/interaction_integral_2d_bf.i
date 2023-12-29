#This is a regression test that exercises the option to include
#the effects of the body force in the interaction integral. This
#uses the same basic model as the other cases in this directory.
#This is a placeholder until a suitable problem with an analytical
#solution can be identified.

[GlobalParams]
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = False
[]

[Mesh]
  file = crack2d.e
  displacements = 'disp_x disp_y'
#  uniform_refine = 3
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
  body_force = body_force
[]

[Modules/TensorMechanics]
  [Master/all]
    strain = FINITE
    add_variables = true
    generate_output = 'stress_xx stress_yy stress_zz vonmises_stress'
    planar_formulation = PLANE_STRAIN
  []
  [MaterialVectorBodyForce/all]
    body_force = body_force
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
    boundary = 700
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
  [body_force]
    type = GenericConstantVectorMaterial
    prop_names = 'body_force'
    prop_values = '0.1 0.1 0.0'
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
