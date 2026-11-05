# Displacement-controlled Hertz test -- full LOAD then UNLOAD cycle.
#
# The rigid contactor is prescribed to move up into the material (t = 0 -> 1)
# then retract back to its start position (t = 1 -> 2).  Unloading is the
# interesting part: LM nodes that were on the gap branch of the min-NCP
# during loading must transition back to the lambda-branch (lambda = 0) as
# the contactor retracts.  This exercises the SSLS + ConstantBounds path's
# active-set update on release, which the loading-only shipping test does
# not touch.

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
    block = 1000
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

[Functions]
  [sphere_up]
    # Triangle wave -- indent to y = -1.99 at t = 1, then release back to
    # y = -2 at t = 2.
    type = PiecewiseLinear
    x = '0  1    2'
    y = '0  0.01 0'
  []
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
    center = '0 -4 0'
    radius = 2.0
    disp_y_function = sphere_up
  []
[]

[Variables]
  [disp_x]
    block = '1 contact_lower'
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
    boundary = 1
    value = 0.0
  []
  [pin_top]
    # Material top pinned to zero -- the contactor moves up into the material
    # instead of the material moving down into the contactor.
    type = DirichletBC
    variable = disp_y
    boundary = 2
    value = 0.0
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
  end_time   = 2.0
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

[Outputs]
  [csv]
    type = CSV
    execute_on = 'TIMESTEP_END'
  []
[]
