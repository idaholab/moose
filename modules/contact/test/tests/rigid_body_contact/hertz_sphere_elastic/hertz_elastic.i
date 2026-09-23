# Rigid-sphere-on-elastic-half-space Hertz test, 2D axisymmetric, small strain.
#
# Uses the analytic-level-set contact stack:
#   * SphereContactor supplies the signed distance g_LS and its normal.
#   * RigidBodyNodalNCPKernel writes  R_lambda_i = min(lambda_i, c * g_LS(x_i + u_i))
#     at each Lagrange-multiplier DoF on the deformable contact sideset's
#     lower-d block.  Pointwise per-node NCP.  No mortar segment mesh, no AD.
#   * RigidBodyNormalMechanicalContact applies -lambda * n * phi_test to the
#     coupled disp equations along the same lower-d block.
#   * PETSc SNESVINEWTONSSLS + ConstantBounds enforces lambda >= 0.
#
# Reuses the deformable body from hertz_contact_rz.e (subdomain 1 = elastic
# sphere, R = 2, curved bottom on sideset 100).  The mesh's original rigid
# indenter (subdomain 1000) is stripped by BlockDeletionGenerator since the
# analytic sphere replaces it.
#
# Analytical Hertz (rigid sphere R = 2 on elastic sphere of same R, both
# geometrically curved, with E = 1.40625e7, nu = 0.25):
#   E* = E / (1 - nu^2) = 1.5e7,   R_eff = 1
#   For delta = 0.01: a = sqrt(R delta) = 0.1,  p0 = 2 E* a / (pi R) = 9.55e5.

[GlobalParams]
  displacements = 'disp_x disp_y'
  large_kinematics = false
[]

[Mesh]
  [file]
    type = FileMeshGenerator
    file = ../../hertz_spherical/hertz_contact_rz.e
  []
  [drop_rigid_indenter]
    type = BlockDeletionGenerator
    input = file
    block = 1000                        # analytic sphere replaces the meshed indenter
  []
  [contact_lower]
    type = LowerDBlockFromSidesetGenerator
    input = drop_rigid_indenter
    sidesets = '100'
    new_block_id = 10001
    new_block_name = contact_lower
  []
  coord_type = RZ
  allow_renumbering = false
[]

[UserObjects]
  [contact_sparsity]
    type = RigidBodyContactSparsity
    lm_variable = normal_lm
    displacements = 'disp_x disp_y'
    boundary = 100
  []
  [sphere]
    type = SphereContactor
    center = '0 -4 0'                 # sphere top at y = -2, tangent to material tip at t = 0
    radius = 2.0
  []
[]

[Variables]
  [disp_x]
    block = '1 contact_lower'         # lower-d block sees disp via shared nodes
  []
  [disp_y]
    block = '1 contact_lower'
  []
  [normal_lm]
    block = contact_lower
  []
[]

[AuxVariables]
  [bounds_dummy]
    family = LAGRANGE
    order = FIRST
    block = contact_lower
  []
[]

[Bounds]
  [lm_lo]
    type = ConstantBounds
    variable = bounds_dummy
    bounded_variable = normal_lm
    bound_type = lower
    bound_value = 0.0
  []
  [lm_hi]
    type = ConstantBounds
    variable = bounds_dummy
    bounded_variable = normal_lm
    bound_type = upper
    bound_value = 1e12
  []
[]

[Kernels]
  [sdx]
    type = TotalLagrangianStressDivergenceAxisymmetricCylindrical
    variable = disp_x
    component = 0
    block = 1
  []
  [sdy]
    type = TotalLagrangianStressDivergenceAxisymmetricCylindrical
    variable = disp_y
    component = 1
    block = 1
  []
[]

[NodalKernels]
  [ncp]
    type = RigidBodyNodalNCPKernel
    variable = normal_lm
    contactor = sphere
    displacements = 'disp_x disp_y'
    block = contact_lower
  []
[]

[Materials]
  [tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1.40625e7
    poissons_ratio = 0.25
    block = 1
  []
  [stress]
    type = ComputeLagrangianLinearElasticStress
    block = 1
  []
  [strain]
    type = ComputeLagrangianStrainAxisymmetricCylindrical
    block = 1
  []
[]

[Functions]
  [top_disp_y]
    type = PiecewiseLinear
    x = '0  1'
    y = '0 -0.01'
  []
[]

[BCs]
  [rb_tx]
    type = RigidBodyNormalMechanicalContact
    variable = disp_x
    lowerd_variable = normal_lm
    boundary = 100
    contactor = sphere
    component = x
    displacements = 'disp_x disp_y'
  []
  [rb_ty]
    type = RigidBodyNormalMechanicalContact
    variable = disp_y
    lowerd_variable = normal_lm
    boundary = 100
    contactor = sphere
    component = y
    displacements = 'disp_x disp_y'
  []
  [symm_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1                      # r = 0 symmetry axis
    value = 0.0
  []
  [top_deform]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 2                      # top of deformable body, pushed down
    function = top_disp_y
  []
[]

[Problem]
  kernel_coverage_check = false
  material_coverage_check = false
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  automatic_scaling = true

  petsc_options_iname = '-snes_type -pc_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'vinewtonssls lu    NONZERO               1e-12'

  line_search = semismooth

  nl_rel_tol = 1e-9
  nl_abs_tol = 1e-8
  nl_max_its = 40
  l_max_its = 200

  start_time = 0.0
  end_time   = 1.0
  dt         = 0.1
[]

[Postprocessors]
  [max_lm]
    type = NodalExtremeValue
    variable = normal_lm
    block = contact_lower
    value_type = max
  []
  [num_nl]
    type = NumNonlinearIterations
  []
  [cumulative_nl]
    type = CumulativeValuePostprocessor
    postprocessor = num_nl
  []
[]

[VectorPostprocessors]
  [contact_lm]
    type = NodalValueSampler
    variable = normal_lm
    block = contact_lower
    sort_by = x
    execute_on = 'TIMESTEP_END'
  []
[]

[Outputs]
  [csv]
    type = CSV
    execute_on = 'TIMESTEP_END'
  []
[]
