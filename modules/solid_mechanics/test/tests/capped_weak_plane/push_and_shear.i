# Dynamic problem with plasticity.
# A column of material (not subject to gravity) has the z-displacement
# of its sides fixed, but the centre of its bottom side is pushed
# upwards.  This causes failure in the bottom elements.
#
# The problem utilises damping in the following way.
# The DynamicStressDivergenceTensors forms the residual
# integral  grad(stress) + zeta*grad(stress-dot)
#     = V/L * elasticity * (du/dx + zeta * dv/dx)
# where V is the elemental volume, and L is the length-scale,
# and u is the displacement, and v is the velocity.
# The InertialForce forms the residual
# integral  density * (accel + eta * velocity)
#     = V * density * (a + eta * v)
# where a is the acceleration.
# So, a damped oscillator description with both these
# kernels looks like
# 0 = V * (density * a + density * eta * v + elasticity * zeta * v / L^2 + elasticity / L^2 * u)
# Critical damping is when the coefficient of v is
# 2 * sqrt(density * elasticity / L^2)
# In the case at hand, density=1E4, elasticity~1E10 (Young is 16GPa),
# L~1 to 10 (in the horizontal or vertical direction), so this coefficient ~ 1E7 to 1E6.
# Choosing eta = 1E3 and zeta = 1E-2 gives approximate critical damping.
# If zeta is high then steady-state is achieved very quickly.
#
# In the case of plasticity, the effective stiffness of the elements
# is significantly less.  Therefore, the above parameters give
# overdamping.
#
# This simulation is a nice example of the irreversable and non-uniqueness
# of simulations involving plasticity.  The result depends on the damping
# parameters and the time stepping.
[Mesh]
  [generated_mesh]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 10
    ny = 1
    nz = 5
    bias_z = 1.5
    xmin = -10
    xmax = 10
    ymin = -10
    ymax = 10
    zmin = -100
    zmax = 0
  []
  [bottomz_middle]
    type = BoundingBoxNodeSetGenerator
    new_boundary = bottomz_middle
    bottom_left = '-1 -1500 -105'
    top_right = '1 1500 -95'
    input = generated_mesh
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  beta = 0.25 # Newmark time integration
  gamma = 0.5 # Newmark time integration
  eta = 1E3 #0.3E4 # higher values mean more damping via density
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[Kernels]
  [DynamicSolidMechanics] # zeta*K*vel + K * disp
    displacements = 'disp_x disp_y disp_z'
    stiffness_damping_coefficient = 1E-2 # higher values mean more damping via stiffness
    hht_alpha = 0 # better nonlinear convergence than for alpha>0
  []
  [inertia_x] # M*accel + eta*M*vel
    type = InertialForce
    use_displaced_mesh = false
    variable = disp_x
    velocity = vel_x
    acceleration = accel_x
  []
  [inertia_y]
    type = InertialForce
    use_displaced_mesh = false
    variable = disp_y
    velocity = vel_y
    acceleration = accel_y
  []
  [inertia_z]
    type = InertialForce
    use_displaced_mesh = false
    variable = disp_z
    velocity = vel_z
    acceleration = accel_z
  []
[]

[BCs]
  [no_x2]
    type = DirichletBC
    variable = disp_x
    boundary = right
    value = 0.0
  []
  [no_x1]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  []
  [no_y1]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  []
  [no_y2]
    type = DirichletBC
    variable = disp_y
    boundary = top
    value = 0.0
  []

  [z_fixed_sides_xmin]
    type = DirichletBC
    variable = disp_z
    boundary = left
    value = 0
  []
  [z_fixed_sides_xmax]
    type = DirichletBC
    variable = disp_z
    boundary = right
    value = 0
  []

  [bottomz]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = bottomz_middle
    function = min(10*t,1)
  []
[]

[AuxVariables]
  [accel_x]
  []
  [vel_x]
  []
  [accel_y]
  []
  [vel_y]
  []
  [accel_z]
  []
  [vel_z]
  []
  [stress_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_xy]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_xz]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_yz]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [strainp_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [strainp_xy]
    order = CONSTANT
    family = MONOMIAL
  []
  [strainp_xz]
    order = CONSTANT
    family = MONOMIAL
  []
  [strainp_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [strainp_yz]
    order = CONSTANT
    family = MONOMIAL
  []
  [strainp_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [straint_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [straint_xy]
    order = CONSTANT
    family = MONOMIAL
  []
  [straint_xz]
    order = CONSTANT
    family = MONOMIAL
  []
  [straint_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [straint_yz]
    order = CONSTANT
    family = MONOMIAL
  []
  [straint_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [f_shear]
    order = CONSTANT
    family = MONOMIAL
  []
  [f_tensile]
    order = CONSTANT
    family = MONOMIAL
  []
  [f_compressive]
    order = CONSTANT
    family = MONOMIAL
  []
  [intnl_shear]
    order = CONSTANT
    family = MONOMIAL
  []
  [intnl_tensile]
    order = CONSTANT
    family = MONOMIAL
  []
  [iter]
    order = CONSTANT
    family = MONOMIAL
  []
  [ls]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [accel_x] # Calculates and stores acceleration at the end of time step
    type = NewmarkAccelAux
    variable = accel_x
    displacement = disp_x
    velocity = vel_x
    execute_on = timestep_end
  []
  [vel_x] # Calculates and stores velocity at the end of the time step
    type = NewmarkVelAux
    variable = vel_x
    acceleration = accel_x
    execute_on = timestep_end
  []
  [accel_y]
    type = NewmarkAccelAux
    variable = accel_y
    displacement = disp_y
    velocity = vel_y
    execute_on = timestep_end
  []
  [vel_y]
    type = NewmarkVelAux
    variable = vel_y
    acceleration = accel_y
    execute_on = timestep_end
  []
  [accel_z]
    type = NewmarkAccelAux
    variable = accel_z
    displacement = disp_z
    velocity = vel_z
    execute_on = timestep_end
  []
  [vel_z]
    type = NewmarkVelAux
    variable = vel_z
    acceleration = accel_z
    execute_on = timestep_end
  []
  [stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 0
    index_j = 0
  []
  [stress_xy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xy
    index_i = 0
    index_j = 1
  []
  [stress_xz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xz
    index_i = 0
    index_j = 2
  []
  [stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
  []
  [stress_yz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yz
    index_i = 1
    index_j = 2
  []
  [stress_zz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zz
    index_i = 2
    index_j = 2
  []
  [strainp_xx]
    type = RankTwoAux
    rank_two_tensor = plastic_strain
    variable = strainp_xx
    index_i = 0
    index_j = 0
  []
  [strainp_xy]
    type = RankTwoAux
    rank_two_tensor = plastic_strain
    variable = strainp_xy
    index_i = 0
    index_j = 1
  []
  [strainp_xz]
    type = RankTwoAux
    rank_two_tensor = plastic_strain
    variable = strainp_xz
    index_i = 0
    index_j = 2
  []
  [strainp_yy]
    type = RankTwoAux
    rank_two_tensor = plastic_strain
    variable = strainp_yy
    index_i = 1
    index_j = 1
  []
  [strainp_yz]
    type = RankTwoAux
    rank_two_tensor = plastic_strain
    variable = strainp_yz
    index_i = 1
    index_j = 2
  []
  [strainp_zz]
    type = RankTwoAux
    rank_two_tensor = plastic_strain
    variable = strainp_zz
    index_i = 2
    index_j = 2
  []
  [straint_xx]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = straint_xx
    index_i = 0
    index_j = 0
  []
  [straint_xy]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = straint_xy
    index_i = 0
    index_j = 1
  []
  [straint_xz]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = straint_xz
    index_i = 0
    index_j = 2
  []
  [straint_yy]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = straint_yy
    index_i = 1
    index_j = 1
  []
  [straint_yz]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = straint_yz
    index_i = 1
    index_j = 2
  []
  [straint_zz]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = straint_zz
    index_i = 2
    index_j = 2
  []
  [f_shear]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 0
    variable = f_shear
  []
  [f_tensile]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 1
    variable = f_tensile
  []
  [f_compressive]
    type = MaterialStdVectorAux
    property = plastic_yield_function
    index = 2
    variable = f_compressive
  []
  [intnl_shear]
    type = MaterialStdVectorAux
    property = plastic_internal_parameter
    index = 0
    variable = intnl_shear
  []
  [intnl_tensile]
    type = MaterialStdVectorAux
    property = plastic_internal_parameter
    index = 1
    variable = intnl_tensile
  []
  [iter]
    type = MaterialRealAux
    property = plastic_NR_iterations
    variable = iter
  []
  [ls]
    type = MaterialRealAux
    property = plastic_linesearch_needed
    variable = ls
  []
[]

[UserObjects]
  [coh]
    type = SolidMechanicsHardeningConstant
    value = 1E6
  []
  [tanphi]
    type = SolidMechanicsHardeningConstant
    value = 0.5
  []
  [tanpsi]
    type = SolidMechanicsHardeningConstant
    value = 0.166666666667
  []
  [t_strength]
    type = SolidMechanicsHardeningConstant
    value = 1E80
  []
  [c_strength]
    type = SolidMechanicsHardeningConstant
    value = 0
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric_isotropic
    C_ijkl = '6.4E9 6.4E9' # young 16MPa, Poisson 0.25
  []
  [strain]
    type = ComputeIncrementalStrain
  []
  [admissible]
    type = ComputeMultipleInelasticStress
    inelastic_models = stress
    perform_finite_strain_rotations = false
  []
  [stress]
    type = CappedWeakPlaneStressUpdate
    cohesion = coh
    tan_friction_angle = tanphi
    tan_dilation_angle = tanpsi
    tensile_strength = t_strength
    compressive_strength = c_strength
    tip_smoother = 0.5E6
    smoothing_tol = 0.5E6
    yield_function_tol = 1E-2
  []
  [density]
    type = GenericConstantMaterial
    block = 0
    prop_names = density
    prop_values = 1E4
  []
[]

[Preconditioning]
  [andy]
    type = SMP
    full = true
    petsc_options = '-snes_converged_reason -snes_linesearch_monitor'
    petsc_options_iname = '-pc_type -pc_asm_overlap -sub_pc_type -ksp_type -ksp_gmres_restart'
    petsc_options_value = ' asm      2              lu            gmres     200'
  []
[]

[Executioner]
  solve_type = 'NEWTON'
  petsc_options = '-snes_converged_reason'
  line_search = bt
  nl_abs_tol = 1E1
  nl_rel_tol = 1e-5
  l_tol = 1E-10

  l_max_its = 100
  nl_max_its = 100

  end_time = 0.5
  dt = 0.1
  type = Transient
[]

[Outputs]
  file_base = push_and_shear
  exodus = true
  csv = true
[]
