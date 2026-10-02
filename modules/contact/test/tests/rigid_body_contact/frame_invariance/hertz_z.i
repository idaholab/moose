# Frame-invariance regression, Z-direction contact.  See hertz_y.i for
# the baseline; this input takes the same mesh, rotates it 90 degrees
# about the x-axis (y -> z mapping) via TransformGenerator, and permutes
# every axis-tagged parameter so the physical problem is identical up to
# that rotation.  All CSV values must match hertz_y.i / hertz_x.i.

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  large_kinematics = false
[]

[Mesh]
  [file]
    type = FileMeshGenerator
    file = ../../hertz_spherical/hertz_contact.e
  []
  [drop_rigid_indenter]
    type = BlockDeletionGenerator
    input = file
    block = 1000
  []
  # Rotate the mesh 90 degrees about x (y -> z mapping):
  #   x_new = x_old,  y_new = -z_old,  z_new = y_old
  # This puts what was the +y contact face at the +z face -- sphere now
  # pushes in +z direction.
  [rotate]
    type = TransformGenerator
    input = drop_rigid_indenter
    transform = ROTATE_WITH_MATRIX
    rotation_matrix = ' 1 0 0   0 0 -1   0 1 0 '
  []
  [contact_lower]
    type = LowerDBlockFromSidesetGenerator
    input = rotate
    sidesets = '100'
    new_block_id = 10001
    new_block_name = contact_lower
  []
  allow_renumbering = false
[]

[UserObjects]
  [contact_sparsity]
    type = RigidBodyContactSparsity
    lm_variable = normal_lm
    displacements = 'disp_x disp_y disp_z'
    boundary = 100
  []
  [sphere]
    type = SphereContactor
    # Rotate (0, -4, 0) -> (0, 0, -4).
    center = '0 0 -4'
    radius = 2.0
    disp_z_scalar = indenter_z
  []
  [nodal_area]
    type = NodalArea
    boundary = 100
    variable = nodal_area
    execute_on = 'INITIAL LINEAR'
  []
[]

[Functions]
  [applied_force]
    type = PiecewiseLinear
    x = '0 1'
    y = '0 1.0e5'
  []
[]

[Variables]
  [disp_x]
    block = '1 contact_lower'
  []
  [disp_y]
    block = '1 contact_lower'
  []
  [disp_z]
    block = '1 contact_lower'
  []
  [normal_lm]
    block = contact_lower
  []
  [indenter_z]
    family = SCALAR
    order = FIRST
    initial_condition = 0.005
  []
[]

[AuxVariables]
  [nodal_area]
    family = LAGRANGE
    order = FIRST
  []
[]

[Kernels]
  [sdx]
    type = TotalLagrangianStressDivergence
    variable = disp_x
    component = 0
    block = 1
  []
  [sdy]
    type = TotalLagrangianStressDivergence
    variable = disp_y
    component = 1
    block = 1
  []
  [sdz]
    type = TotalLagrangianStressDivergence
    variable = disp_z
    component = 2
    block = 1
  []
[]

[NodalKernels]
  [ncp]
    type = RigidBodyNodalNCPKernel
    variable = normal_lm
    contactor = sphere
    displacements = 'disp_x disp_y disp_z'
    block = contact_lower
    c = 1.0
  []
[]

[ScalarKernels]
  [load_control]
    type = RigidBodyLoadControl
    variable = indenter_z
    boundary = 100
    force = applied_force
    contactor = sphere
    nodal_area = nodal_area
    lm_variable = normal_lm
    displacements = 'disp_x disp_y disp_z'
    # Rotate (0, 1, 0) -> (0, 0, 1).
    direction = '0 0 1'
    c = 1.0
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
    type = ComputeLagrangianStrain
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
    displacements = 'disp_x disp_y disp_z'
  []
  [rb_ty]
    type = RigidBodyNormalMechanicalContact
    variable = disp_y
    lowerd_variable = normal_lm
    boundary = 100
    contactor = sphere
    component = y
    displacements = 'disp_x disp_y disp_z'
  []
  [rb_tz]
    type = RigidBodyNormalMechanicalContact
    variable = disp_z
    lowerd_variable = normal_lm
    boundary = 100
    contactor = sphere
    component = z
    displacements = 'disp_x disp_y disp_z'
  []
  # boundary 1 was an x-plane pinning disp_x; unchanged after rotating y->z.
  # boundary 2 was a y-plane pinning disp_y; after rotation it is a z-plane,
  # so we pin disp_z.  boundary 3 was a z-plane pinning disp_z; after
  # rotation it is a y-plane, so we pin disp_y.
  [symm_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  []
  [symm_z]
    type = DirichletBC
    variable = disp_y
    boundary = 3
    value = 0.0
  []
  [pin_top]
    type = DirichletBC
    variable = disp_z
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
  petsc_options_value = 'newtonls    lu       NONZERO               1e-12'
  line_search = basic

  nl_rel_tol = 1e-9
  nl_abs_tol = 1e-8
  nl_max_its = 40
  l_max_its = 200

  start_time = 0.0
  end_time   = 0.1
  dt = 0.05
[]

[Postprocessors]
  [applied_force]
    type = FunctionValuePostprocessor
    function = applied_force
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [indenter_translation]
    type = ScalarVariable
    variable = indenter_z
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [max_lm]
    type = NodalExtremeValue
    variable = normal_lm
    block = contact_lower
    value_type = max
    execute_on = 'INITIAL TIMESTEP_END'
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
