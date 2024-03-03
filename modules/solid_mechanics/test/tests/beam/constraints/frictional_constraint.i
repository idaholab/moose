# Test for frictional beam constraint.
#
# Using a simple L-shaped geometry with a frictional constraint at the
# corner between the two beams. The longer beam properties and loading is
# taken from an earlier beam regression test for static loading. The maximum
# applied load of 50000 lb should result in a displacement of 3.537e-3. Since
# the constraint is frictional with a low normal force (1.0) and coefficient
# of friction (0.05) and the short beam is much less stiff, the
# y-dir displacement of the long beam is still 3.537e-3. However, the y-dir
# displacement of the short beam increases until the force exceeds the
# frictional capacity which in this case is 0.05 and then remains constant
# after that point.

[Mesh]
  file = beam_cons_patch.e
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_z]
    order = FIRST
    family = LAGRANGE
  [../]
  [./rot_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./rot_y]
    order = FIRST
    family = LAGRANGE
  [../]
  [./rot_z]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[BCs]
  [./fixx1]
    type = DirichletBC
    variable = disp_x
    boundary = '1001 1003'
    value = 0.0
  [../]
  [./fixy1]
    type = DirichletBC
    variable = disp_y
    boundary = '1001 1003'
    value = 0.0
  [../]
  [./fixz1]
    type = DirichletBC
    variable = disp_z
    boundary = '1001 1003'
    value = 0.0
  [../]
  [./fixr1]
    type = DirichletBC
    variable = rot_x
    boundary = '1001 1003'
    value = 0.0
  [../]
  [./fixr2]
    type = DirichletBC
    variable = rot_y
    boundary = '1001 1003'
    value = 0.0
  [../]
  [./fixr3]
    type = DirichletBC
    variable = rot_z
    boundary = '1001 1003'
    value = 0.0
  [../]
[]

[Constraints]
  [./tie_y_fuel]
    type = NodalFrictionalConstraint
    normal_force = 1.0
    tangential_penalty = 1.2e5
    friction_coefficient = 0.05
    boundary = 1005
    secondary = 1004
    variable = disp_y
  [../]
  [./tie_x_fuel]
    type = NodalStickConstraint
    penalty = 1.2e14
    boundary = 1005
    secondary = 1004
    variable = disp_x
  [../]
  [./tie_z_fuel]
    type = NodalStickConstraint
    penalty = 1.2e14
    boundary = 1005
    secondary = 1004
    variable = disp_z
  [../]
  [./tie_rot_y_fuel]
    type = NodalStickConstraint
    penalty = 1.2e14
    boundary = 1005
    secondary = 1004
    variable = rot_y
  [../]
  [./tie_rot_x_fuel]
    type = NodalStickConstraint
    penalty = 1.2e14
    boundary = 1005
    secondary = 1004
    variable = rot_x
  [../]
  [./tie_rot_z_fuel]
    type = NodalStickConstraint
    penalty = 1.2e14
    boundary = 1005
    secondary = 1004
    variable = rot_z
  [../]
[]

[Functions]
  [./force_loading]
    type = PiecewiseLinear
    x = '0.0 5.0'
    y = '0.0 50000.0'
  [../]
[]

[NodalKernels]
  [./force_x2]
    type = UserForcingFunctionNodalKernel
    variable = disp_y
    boundary = '1004'
    function = force_loading
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]
[Executioner]
  type = Transient
  solve_type = PJFNK
  line_search = 'none'
  nl_max_its = 15
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-8

  dt = 1
  dtmin = 1
  end_time = 5
[]

[Kernels]
  [./solid_disp_x]
    type = StressDivergenceBeam
    block = '1 2'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 0
    variable = disp_x
  [../]
  [./solid_disp_y]
    type = StressDivergenceBeam
    block = '1 2'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 1
    variable = disp_y
  [../]
  [./solid_disp_z]
    type = StressDivergenceBeam
    block = '1 2'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 2
    variable = disp_z
  [../]
  [./solid_rot_x]
    type = StressDivergenceBeam
    block = '1 2'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 3
    variable = rot_x
  [../]
  [./solid_rot_y]
    type = StressDivergenceBeam
    block = '1 2'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 4
    variable = rot_y
  [../]
  [./solid_rot_z]
    type = StressDivergenceBeam
    block = '1 2'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 5
    variable = rot_z
  [../]
[]

[Materials]
  [./elasticity_pipe]
    type = ComputeElasticityBeam
    shear_coefficient = 1.0
    youngs_modulus = 30e6
    poissons_ratio = 0.3
    block = 1
    outputs = exodus
    output_properties = 'material_stiffness material_flexure'
  [../]
  [./strain_pipe]
    type = ComputeIncrementalBeamStrain
    block = '1'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    area = 28.274
    Ay = 0.0
    Az = 0.0
    Iy = 1.0
    Iz = 1.0
    y_orientation = '0.0 0.0 1.0'
  [../]
  [./stress_pipe]
    type = ComputeBeamResultants
    block = 1
    outputs = exodus
    output_properties = 'forces moments'
  [../]
  [./elasticity_cons]
    type = ComputeElasticityBeam
    shear_coefficient = 1.0
    youngs_modulus = 10e2
    poissons_ratio = 0.3
    block = 2
    outputs = exodus
    output_properties = 'material_stiffness material_flexure'
  [../]
  [./strain_cons]
    type = ComputeIncrementalBeamStrain
    block = '2'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    area = 1.0
    Ay = 0.0
    Az = 0.0
    Iy = 1.0
    Iz = 1.0
    y_orientation = '0.0 0.0 1.0'
  [../]
  [./stress_cons]
    type = ComputeBeamResultants
    block = 2
    outputs = exodus
    output_properties = 'forces moments'
  [../]
[]

[Postprocessors]
  [./disp_y_n4]
    type = NodalVariableValue
    variable = disp_y
    nodeid = 3
  [../]
  [./disp_y_n2]
    type = NodalVariableValue
    variable = disp_y
    nodeid = 1
  [../]
  [./horz_forces_y]
    type = PointValue
    point = '9.9 60.0 0.0'
    variable = forces_y
  [../]
  [./forces_y]
    type = PointValue
    point = '10.0 59.9 0.0'
    variable = forces_y
  [../]
[]

[Outputs]
  csv = true
  exodus = true
[]
